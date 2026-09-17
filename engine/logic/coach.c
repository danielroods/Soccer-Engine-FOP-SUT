#include "coach.h"
#include "core/constants.h"
#include "entities/ball.h"
#include "entities/team.h"
#include "game/scene.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

// Set to false to let the other team use their own logic (if you implement it)
// Set to true to test your logic on both teams
bool coach_both_teams = false;

/* -------------------------------------------------------------------------
 * Logic Functions
 *  TODO 1: You must implement the following functions in Phase 2.
 *        Each player in each team has its own functions.
 *        You can add new functions, but are NOT ALLOWED to remove
 *        the existing functions or change their structure.
 * ------------------------------------------------------------------------- 
 * ⚠️ STUDENT RULES FOR PHASE 2:
 * You are restricted to modifying ONLY specific variables in each function:
 *
 * 1. MOVEMENT FUNCTIONS (movement_logic_X_Y):
 * Allowed: player->velocity
 * Goal:    Determine the direction and speed of movement.
 *
 * 2. SHOOTING FUNCTIONS (shooting_logic_X_Y):
 * Allowed: ball->velocity
 * Goal:    Determine the direction and power of the kick/pass.
 *
 * 3. CHANGE STATE FUNCTIONS (change_state_logic_X_Y):
 * Allowed: player->state
 * Goal:    Switch between IDLE, MOVING, SHOOTING, or INTERCEPTING.
 *
 * NOTE: Directly modifying any other attributes will be flagged as a violation.
 * Thank you for your attention to this matter!
 * ------------------------------------------------------------------------- */
void move_to_home(struct Player *self) {
    struct Vec2 home = get_positions(self->team, self->kit);
    float dx = home.x - self->position.x;
    float dy = home.y - self->position.y;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist > 5.0f) {
        self->velocity.x = (dx / dist) * self->talents.agility * 10;
        self->velocity.y = (dy / dist) * self->talents.agility * 10;
    }
    else {
        self->velocity.x = 0;
        self->velocity.y = 0;
    }
}
bool is_closest_teammate(struct Player *self, struct Scene *scene) {
    struct Team *my_team = (self->team == 1) ? scene->first_team : scene->second_team;
    float my_dx = scene->ball->position.x - self->position.x;
    float my_dy = scene->ball->position.y - self->position.y;
    float my_dist = sqrt(my_dx * my_dx + my_dy * my_dy);
    for (int i = 0; i < PLAYER_COUNT; i++) {
        struct Player *teammate = my_team->players[i];
        if (!teammate || teammate == self) continue;
        float t_dx = scene->ball->position.x - teammate->position.x;
        float t_dy = scene->ball->position.y - teammate->position.y;
        float t_dist = sqrt(t_dx * t_dx + t_dy * t_dy);
        if (t_dist < my_dist) return false;
        if (t_dist == my_dist && teammate->kit < self->kit) return false;
    }
    return true;
}
bool handle_out_and_restarts(struct Player *self, struct Scene *scene) {
    if (scene->state != STATE_OUT && scene->state != STATE_RESTARTING) {
        return false;
    }
    struct Player *kicker = scene->ball->possessor;
    if (kicker == NULL) {
        if (is_closest_teammate(self, scene)) {
            kicker = self;
        }
    }
    if (self == kicker) {
        self->velocity.x = 0;
        self->velocity.y = 0;
    }
    else if (kicker != NULL && self->team != kicker->team) {
        move_to_home(self);
    }
    else {
        move_to_home(self);
    }
    return true;
}

void smart_goalkeeper_move(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    float ball_x = scene->ball->position.x;
    float ball_y = scene->ball->position.y;
    struct Vec2 home = get_positions(self->team, self->kit);
    float target_x = home.x;
    float target_y = ball_y;
    float goal_top = CENTER_Y - (GOAL_HEIGHT / 2.5f);
    float goal_bottom = CENTER_Y + (GOAL_HEIGHT / 2.5f);
    if (target_y < goal_top) target_y = goal_top;
    if (target_y > goal_bottom) target_y = goal_bottom;
    float dx = ball_x - self->position.x;
    float dy = ball_y - self->position.y;
    float dist = sqrt(dx*dx + dy*dy);
    if (dist < 65.0f) {
        target_x = ball_x;
        if (self->team == 1 && target_x > home.x + 65.0f) target_x = home.x + 65.0f;
        if (self->team == 1 && target_x < home.x - 65.0f) target_x = home.x - 65.0f;
        if (self->team == 2 && target_x > home.x + 65.0f) target_x = home.x + 65.0f;
        if (self->team == 2 && target_x < home.x - 65.0f) target_x = home.x - 65.0f;
        target_y = ball_y;
    }
    float move_dx = target_x - self->position.x;
    float move_dy = target_y - self->position.y;
    float move_dist = sqrt(move_dx*move_dx + move_dy*move_dy);
    if (move_dist > 1.0f) {
        self->velocity.x = (move_dx / move_dist) * self->talents.agility * 10;
        self->velocity.y = (move_dy / move_dist) * self->talents.agility * 10;
    } else {
        self->velocity.x = 0;
        self->velocity.y = 0;
    }
}
void smart_shoot(struct Player *self, struct Scene *scene, int role) {
    float max_power = self->talents.shooting * 35.0f;

    int is_kickoff = (self->kit == 0 &&
                      fabs(scene->ball->position.x - CENTER_X) < 10.0f &&
                      fabs(scene->ball->position.y - CENTER_Y) < 10.0f &&
                      fabs(scene->ball->velocity.x) < 1.0f &&
                      fabs(scene->ball->velocity.y) < 1.0f);

    if (role == 3) {
        float target_x = CENTER_X;
        float target_y = (scene->ball->position.y > CENTER_Y) ? (PITCH_Y - 90.0f) : (PITCH_Y + PITCH_H + 90.0f);
        float dx = target_x - scene->ball->position.x;
        float dy = target_y - scene->ball->position.y;
        float d = sqrt(dx*dx + dy*dy);
        if (d > 0.1f) {
            scene->ball->velocity.x = (dx / d) * max_power;
            scene->ball->velocity.y = (dy / d) * max_power;
        }
        return;
    }

    struct Player *best_teammate = NULL;
    float best_distance = 2000.0f;
    int has_target = 0;
    struct Team *my_team = (self->team == 1) ? scene->first_team : scene->second_team;

    for(int i = 0; i < PLAYER_COUNT; i++) {
        struct Player *teammate = my_team->players[i];
        if(teammate == self || !teammate) continue;

        int valid = 0;
        if(is_kickoff) {
            valid = (self->team == 1) ? (teammate->position.x < self->position.x) : (teammate->position.x > self->position.x);
        }
        else {
            valid = (self->team == 1) ? (teammate->position.x > self->position.x) : (teammate->position.x < self->position.x);
        }

        if(valid) {
            float dx = teammate->position.x - scene->ball->position.x;
            float dy = teammate->position.y - scene->ball->position.y;
            float distance = sqrt(dx*dx + dy*dy);
            if(distance < best_distance) {
                best_distance = distance;
                best_teammate = teammate;
                has_target = 1;
            }
        }
    }

    if(has_target && best_distance > 0.1f &&
       ((self->team == 1 && scene->ball->position.x < CENTER_X + 220.0f) ||
        (self->team == 2 && scene->ball->position.x > CENTER_X - 220.0f))) {
        float dx = best_teammate->position.x - scene->ball->position.x;
        float dy = best_teammate->position.y - scene->ball->position.y;
        scene->ball->velocity.x = (dx / best_distance) * max_power;
        scene->ball->velocity.y = (dy / best_distance) * max_power;
    }
    else {
        float target_x = (self->team == 1) ? PITCH_X + PITCH_W : PITCH_X;
        if (is_kickoff) target_x = (self->team == 1) ? PITCH_X : PITCH_X + PITCH_W;

        float target_y = (scene->ball->position.y > CENTER_Y) ? CENTER_Y - 25.0f : CENTER_Y + 25.0f;
        float dx = target_x - scene->ball->position.x;
        float dy = target_y - scene->ball->position.y;
        float d = sqrt(dx*dx + dy*dy);

        float dist_to_goal = fabs(target_x - scene->ball->position.x);
        int under_pressure = 0;
        struct Team *enemy_team = (self->team == 1) ? scene->second_team : scene->first_team;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            struct Player *enemy = enemy_team->players[i];
            if (!enemy || enemy->state == IDLE) continue;
            float edx = enemy->position.x - self->position.x;
            float edy = enemy->position.y - self->position.y;
            if (sqrt(edx*edx + edy*edy) < 100.0f) {
                under_pressure = 1;
                break;
            }
        }

        if (is_kickoff) {
            scene->ball->velocity.x = (dx / d) * max_power;
            scene->ball->velocity.y = (dy / d) * max_power;
        }
        else if (dist_to_goal < 300.0f || under_pressure) {
            scene->ball->velocity.x = (dx / d) * max_power;
            scene->ball->velocity.y = (dy / d) * max_power;
        }
        else if (d > 0.1f) {
            float player_speed = self->talents.agility * 10.0f;
            scene->ball->velocity.x = (dx / d) * (player_speed * 1.05f);
            scene->ball->velocity.y = (dy / d) * (player_speed * 1.05f);
        }
        else {
            scene->ball->velocity.x = 0;
            scene->ball->velocity.y = 0;
        }
    }
}
void smart_move(struct Player *self, struct Scene *scene, float attack_radius, int role) {
    if (scene->state == STATE_OUT || scene->state == STATE_RESTARTING) {
        move_to_home(self);
        return;
    }
    if (scene->ball->possessor == self) {
        float target_x = (self->team == 1) ? PITCH_X + PITCH_W : PITCH_X;
        float target_y = CENTER_Y;
        float dx = target_x - self->position.x;
        float dy = target_y - self->position.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 1.0f) {
            self->velocity.x = (dx / dist) * self->talents.agility * 10.0f;
            self->velocity.y = (dy / dist) * self->talents.agility * 10.0f;
        }
        else {
            self->velocity.x = 0;
            self->velocity.y = 0;
        }
        return;
    }

    float ball_x = scene->ball->position.x;
    float ball_y = scene->ball->position.y;
    float dx = ball_x - self->position.x;
    float dy = ball_y - self->position.y;
    float dist_to_ball = sqrt(dx * dx + dy * dy);
    float center_line = PITCH_X + (PITCH_W / 2.0f);
    bool should_chase = false;
    bool teammate_has_ball = (scene->ball->possessor != NULL && scene->ball->possessor->team == self->team && scene->ball->possessor != self);
    if (dist_to_ball < attack_radius && is_closest_teammate(self, scene) && !teammate_has_ball) {
        should_chase = true;
    }

    if (role == 1) {
        bool ball_in_my_half = (self->team == 1) ? (ball_x < center_line) : (ball_x > center_line);
        if (!ball_in_my_half) should_chase = false;
    }

    if (role == 2 && dist_to_ball < 120.0f && is_closest_teammate(self, scene)) {
        should_chase = true;
    }
    if (teammate_has_ball) {
        should_chase = false;
    }

    if (should_chase) {
        if (dist_to_ball > 0.1f) {
            self->velocity.x = (dx / dist_to_ball) * self->talents.agility * 10;
            self->velocity.y = (dy / dist_to_ball) * self->talents.agility * 10;
        }
        else {
            self->velocity.x = 0; self->velocity.y = 0;
        }
    }
    else {
        struct Vec2 home = get_positions(self->team, self->kit);
        float target_x, target_y;

        if (role == 0) {
            float support_dist = (self->team == 1) ? -80.0f : 80.0f;
            target_x = ball_x + support_dist;
            if (self->team == 1 && target_x < home.x) target_x = home.x + (ball_x - home.x) * 0.4f;
            if (self->team == 2 && target_x > home.x) target_x = home.x + (ball_x - home.x) * 0.4f;
            target_y = (home.y * 0.5f) + (ball_y * 0.5f);
        }
        else if (role == 2) {
            bool ball_in_opp_half = (self->team == 1) ? (ball_x > center_line) : (ball_x < center_line);
            if (ball_in_opp_half) {
                float base_run = (self->team == 1) ? 150.0f : -150.0f;
                float x_offset = (self->team == 1) ? (home.x - (center_line - 150.0f)) : (home.x - (center_line + 150.0f));
                target_x = ball_x + base_run + (x_offset * 0.8f);
                if (self->team == 1 && target_x > PITCH_X + PITCH_W - 50) target_x = PITCH_X + PITCH_W - 50;
                if (self->team == 2 && target_x < PITCH_X + 50) target_x = PITCH_X + 50;
                float y_offset = home.y - CENTER_Y;
                if (y_offset == 0.0f) target_y = ball_y + ((self->kit == 0) ? -60.0f : 60.0f);
                else target_y = ball_y + (y_offset * 0.6f);
            }
            else {
                target_x = home.x + (ball_x - home.x) * 0.3f;
                if (self->team == 1 && target_x < center_line - 20) target_x = center_line - 20;
                if (self->team == 2 && target_x > center_line + 20) target_x = center_line + 20;
                target_y = (home.y * 0.7f) + (ball_y * 0.3f);
            }
        }
        else if (role == 4) {
            if (self->team == 1) target_x = PITCH_X + (PITCH_W * 0.75f);
            else target_x = PITCH_X + (PITCH_W * 0.25f);
            target_y = (ball_y < CENTER_Y) ? ball_y + 20 : ball_y - 20;
            if (target_y < PITCH_Y + 50) target_y = PITCH_Y + 50;
            if (target_y > PITCH_Y + PITCH_H - 50) target_y = PITCH_Y + PITCH_H - 50;
        }
        else {
            target_x = home.x;
            bool ball_is_behind = (self->team == 1) ? (ball_x < home.x) : (ball_x > home.x);
            if (ball_is_behind) target_x = ball_x;
            target_y = home.y + (ball_y - home.y) * 0.2f;
            if (self->team == 1 && target_x > center_line - 60) target_x = center_line - 60;
            if (self->team == 2 && target_x < center_line + 60) target_x = center_line + 60;
        }

        float h_dx = target_x - self->position.x;
        float h_dy = target_y - self->position.y;
        float h_dist = sqrt(h_dx * h_dx + h_dy * h_dy);

        if (h_dist > 2.0f) {
            self->velocity.x = (h_dx / h_dist) * self->talents.agility * 7;
            self->velocity.y = (h_dy / h_dist) * self->talents.agility * 7;
        } else {
            self->velocity.x = 0; self->velocity.y = 0;
        }
    }
}
void smart_change_state(struct Player *self, struct Scene *scene) {
    float dx = scene->ball->position.x - self->position.x;
    float dy = scene->ball->position.y - self->position.y;
    float distance = sqrt(dx*dx + dy*dy);

    bool physically_has_ball = (scene->ball->possessor == self && distance < 35.0f);
    bool teammate_has_ball = (scene->ball->possessor != NULL && scene->ball->possessor->team == self->team && scene->ball->possessor != self);
    bool enemy_has_ball = (scene->ball->possessor != NULL && scene->ball->possessor->team != self->team);

    if (scene->state == STATE_OUT || scene->state == STATE_RESTARTING) {
        if (physically_has_ball) self->state = SHOOTING;
        else self->state = MOVING;
        return;
    }

    if (physically_has_ball) {
        if (self->state == SHOOTING) self->state = MOVING;
        else self->state = SHOOTING;
    }
    else if (teammate_has_ball) {
        self->state = MOVING;
    }
    else {
        int is_near_sideline = (scene->ball->position.y < PITCH_Y + 15.0f ||
                                scene->ball->position.y > PITCH_Y + PITCH_H - 15.0f ||
                                scene->ball->position.x < PITCH_X + 15.0f ||
                                scene->ball->position.x > PITCH_X + PITCH_W - 15.0f);

        if (enemy_has_ball && is_near_sideline && distance < 70.0f) {
            self->state = MOVING;
        }
        else if (distance < 60.0f) {
            self->state = INTERCEPTING;
        }
        else {
            self->state = MOVING;
        }
    }
}
/* Team 1 movement logic */
void movement_logic_1_0(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 800.0f, 2);
}
void movement_logic_1_1(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 300.0f, 1);
    return;
}
void movement_logic_1_2(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 300.0f, 1);
    return;
}
void movement_logic_1_3(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 600.0f, 0);
    return;
}
void movement_logic_1_4(struct Player *self, struct Scene *scene) {
    smart_goalkeeper_move(self, scene);
}
void movement_logic_1_5(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 800.0f, 4);
}

/* Team 2 movement logic */
void movement_logic_2_0(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 800.0f, 2);
}
void movement_logic_2_1(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 300.0f, 1);
    return;
}
void movement_logic_2_2(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 300.0f, 1);
    return;
}
void movement_logic_2_3(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 600.0f, 0);
    return;
}
void movement_logic_2_4(struct Player *self, struct Scene *scene) {
    smart_goalkeeper_move(self, scene);
}
void movement_logic_2_5(struct Player *self, struct Scene *scene) {
    if (handle_out_and_restarts(self, scene)) {
        return;
    }
    smart_move(self, scene, 800.0f, 4);
    return;
}

/* Team 1 shooting logic */
void shooting_logic_1_0(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_1_1(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_1_2(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene, 0);
}
void shooting_logic_1_3(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_1_4(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,3);
}
void shooting_logic_1_5(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}

/* Team 2 shooting logic */
void shooting_logic_2_0(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_2_1(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_2_2(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_2_3(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}
void shooting_logic_2_4(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene, 3);
}
void shooting_logic_2_5(struct Player *self, struct Scene *scene) {
    smart_shoot(self, scene,0);
}

/* Team 1 change_state logic */
void change_state_logic_1_0(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_1_1(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_1_2(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_1_3(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_1_4(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_1_5(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}

/* Team 2 change_state logic */
void change_state_logic_2_0(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_2_1(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_2_2(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_2_3(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_2_4(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}
void change_state_logic_2_5(struct Player *self, struct Scene *scene) {
    smart_change_state(self, scene);
}

/* -------------------------------------------------------------------------
 * Lookup tables for factory
 * ------------------------------------------------------------------------- */
static PlayerLogicFn team1_movement[6] = {
    movement_logic_1_0, movement_logic_1_1, movement_logic_1_2,
    movement_logic_1_3, movement_logic_1_4, movement_logic_1_5
};

static PlayerLogicFn team2_movement[6] = {
    movement_logic_2_0, movement_logic_2_1, movement_logic_2_2,
    movement_logic_2_3, movement_logic_2_4, movement_logic_2_5
};

static PlayerLogicFn team1_shooting[6] = {
    shooting_logic_1_0, shooting_logic_1_1, shooting_logic_1_2,
    shooting_logic_1_3, shooting_logic_1_4, shooting_logic_1_5
};

static PlayerLogicFn team2_shooting[6] = {
    shooting_logic_2_0, shooting_logic_2_1, shooting_logic_2_2,
    shooting_logic_2_3, shooting_logic_2_4, shooting_logic_2_5
};

static PlayerLogicFn team1_change_state[6] = {
    change_state_logic_1_0, change_state_logic_1_1, change_state_logic_1_2,
    change_state_logic_1_3, change_state_logic_1_4, change_state_logic_1_5
};

static PlayerLogicFn team2_change_state[6] = {
    change_state_logic_2_0, change_state_logic_2_1, change_state_logic_2_2,
    change_state_logic_2_3, change_state_logic_2_4, change_state_logic_2_5
};

/* -------------------------------------------------------------------------
 * Factory functions
 * ------------------------------------------------------------------------- */
PlayerLogicFn get_movement_logic(int team, int kit) {
    if (coach_both_teams) return team1_movement[kit];
    return (team == 1) ? team1_movement[kit] : team2_movement[kit];
}

PlayerLogicFn get_shooting_logic(int team, int kit) {
    if (coach_both_teams) return team1_shooting[kit];
    return (team == 1) ? team1_shooting[kit] : team2_shooting[kit];
}

PlayerLogicFn get_change_state_logic(int team, int kit) {
    if (coach_both_teams) return team1_change_state[kit];
    return (team == 1) ? team1_change_state[kit] : team2_change_state[kit];
}

/* -------------------------------------------------------------------------
 * TALENTS
 *  TODO 2: Replace these default values with your desired skill points.
 * ------------------------------------------------------------------------- */
/* Team 1 */
static struct Talents team1_talents[6] = {
    {4, 6, 4, 6},
    {6, 6, 4, 4},
    {6, 6, 4, 4},
    {5, 6, 4, 5},
    {3, 7, 1, 7},
    {1, 7, 3, 9},
};

/* Team 2 */
static struct Talents team2_talents[6] = {
    {4, 4, 3, 6},
    {6, 7, 3, 4},
    {6, 6, 4, 4},
    {5, 6, 4, 5},
    {3, 9, 1, 7},
    {1, 7, 4, 8},
};

struct Talents get_talents(int team, int kit) {
    if (coach_both_teams) return team1_talents[kit];
    return (team == 1) ? team1_talents[kit] : team2_talents[kit];
}


/* -------------------------------------------------------------------------
 * Positioning
 *  TODO 3: Decide players positions at kick-off.
 *        Players must stay on their half, outside the center circle.
 *        Keep in mind that the kick-off team's first player will automatically
 *             be placed at the center of the pitch.
 * ------------------------------------------------------------------------- */
/* Team 1 */
static struct Vec2 team1_positions[6] = {
    {CENTER_X - 150, CENTER_Y -150},
    {CENTER_X - 300, CENTER_Y-150},
    {CENTER_X -300 , CENTER_Y+150},
    {CENTER_X -200, CENTER_Y},
    {PITCH_X + 30, CENTER_Y},
    {CENTER_X - 150, CENTER_Y+150},
};

/* Team 2 */
static struct Vec2 team2_positions[6] = {
    {CENTER_X + 150, CENTER_Y - 150},
    {CENTER_X + 300, CENTER_Y - 150},
    {CENTER_X + 300, CENTER_Y + 150},
    {CENTER_X + 200, CENTER_Y},
    {PITCH_X + PITCH_W - 30, CENTER_Y},
    {CENTER_X + 150, CENTER_Y + 150},
};

struct Vec2 get_positions(int team, int kit) {
    return (team == 1) ? team1_positions[kit] : team2_positions[kit];
}