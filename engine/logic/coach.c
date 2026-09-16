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



struct Vec2 get_positions(int team, int kit);

static bool am_i_closest_to_ball(struct Player *self, struct Scene *scene) {
    struct Team *my_team = (self->team == 1) ? scene->first_team : scene->second_team;
    float my_dist = hypotf(scene->ball->position.x - self->position.x, scene->ball->position.y - self->position.y);

    for (int i = 0; i < 6; i++) {
        struct Player *teammate = my_team->players[i];
        if (teammate && teammate != self && i != 0) { 
            float their_dist = hypotf(scene->ball->position.x - teammate->position.x, scene->ball->position.y - teammate->position.y);
            if (their_dist < my_dist) {
                return false;
            }
        }
    }
    return true;
}

static void smart_movement(struct Player *self, struct Scene *scene, int kit) {
    float target_x, target_y;

    if (am_i_closest_to_ball(self, scene)) {
        target_x = scene->ball->position.x;
        target_y = scene->ball->position.y;
    } else {
        struct Vec2 home = get_positions(self->team, kit);
        
        float shift_x = (scene->ball->position.x - CENTER_X) * 0.3f;
        
        target_x = home.x + shift_x;
        target_y = home.y;
        
        if (self->team == 1 && target_x > 800.0f) target_x = 800.0f;
        if (self->team == 2 && target_x < 200.0f) target_x = 200.0f;
    }

    float dx = target_x - self->position.x;
    float dy = target_y - self->position.y;
    float dist = hypotf(dx, dy);

    if (dist > 5.0f) {
        float speed = ((float)self->talents.agility / 10.0f) * 100.0f;
        self->velocity.x = (dx / dist) * speed;
        self->velocity.y = (dy / dist) * speed;
    } else {
        self->velocity.x = 0.0f;
        self->velocity.y = 0.0f;
    }
}


/* Team 1 movement logic */
void movement_logic_1_0(struct Player *self, struct Scene *scene) { 
    float dy = scene->ball->position.y - self->position.y;
    float dist = fabsf(dy);
    if (dist > 1.0f && scene->ball->position.x < CENTER_X + 100.0f) {
        float speed = ((float)self->talents.agility / 10.0f) * 100.0f;
        self->velocity.x = 0.0f;
        self->velocity.y = (dy / dist) * speed;
    } else {
        self->velocity.x = 0.0f;
        self->velocity.y = 0.0f;
    }
}
void movement_logic_1_1(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 1); }
void movement_logic_1_2(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 2); }
void movement_logic_1_3(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 3); }
void movement_logic_1_4(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 4); }
void movement_logic_1_5(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 5); }

/* Team 2 movement logic */
void movement_logic_2_0(struct Player *self, struct Scene *scene) {
    float dy = scene->ball->position.y - self->position.y;
    float dist = fabsf(dy);
    if (dist > 1.0f && scene->ball->position.x > CENTER_X - 100.0f) {
        float speed = ((float)self->talents.agility / 10.0f) * 100.0f;
        self->velocity.x = 0.0f;
        self->velocity.y = (dy / dist) * speed;
    } else {
        self->velocity.x = 0.0f;
        self->velocity.y = 0.0f;
    }
}
void movement_logic_2_1(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 1); }
void movement_logic_2_2(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 2); }
void movement_logic_2_3(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 3); }
void movement_logic_2_4(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 4); }
void movement_logic_2_5(struct Player *self, struct Scene *scene) { smart_movement(self, scene, 5); }



///////////////////////////////////////////////////////////////////////

static void pass_to_teammate(struct Player *self, struct Scene *scene, int target_kit) {
    struct Ball *ball = scene->ball;
    
    float target_x = (self->team == 1) ? 600.0f : 400.0f;
    float target_y = (target_kit % 2 == 0) ? CENTER_Y - 100.0f : CENTER_Y + 100.0f;

    float dx = target_x - ball->position.x;
    float dy = target_y - ball->position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    
    if (dist > 0.0f) {
        float pass_power = ((float)self->talents.shooting / 10.0f) * (MAX_BALL_VELOCITY * 0.6f); 
        ball->velocity.x = (dx / dist) * pass_power;
        ball->velocity.y = (dy / dist) * pass_power;
    }
}

static void shoot_to_goal(struct Player *self, struct Scene *scene) {
    struct Ball *ball = scene->ball;
    float target_x = (self->team == 1) ? 970.0f : 30.0f;
    
    float target_y;
    if (self->position.y > CENTER_Y) {
        target_y = CENTER_Y - 85.0f;
    } else {
        target_y = CENTER_Y + 85.0f;
    }

    float dx = target_x - ball->position.x;
    float dy = target_y - ball->position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist > 0.0f) {
        float power = ((float)self->talents.shooting / 10.0f) * MAX_BALL_VELOCITY;
        ball->velocity.x = (dx / dist) * power;
        ball->velocity.y = (dy / dist) * power;
    }
}


void smart_kick(struct Player *self, struct Scene *scene) {
    if (scene->ball->velocity.x == 0.0f && scene->ball->velocity.y == 0.0f) {
        if (self->team == 1) {
            scene->ball->velocity.x = -150.0f;
            scene->ball->velocity.y = 80.0f;
        } else {
            scene->ball->velocity.x = 150.0f;
            scene->ball->velocity.y = -80.0f;
        }
        return;
    }

    struct Ball *ball = scene->ball;
    
    float goal_x = (self->team == 1) ? 970.0f : 30.0f;
    float dist_to_goal = fabsf(goal_x - self->position.x);

    bool should_shoot = (dist_to_goal < 300.0f);

    float target_x, target_y, power_factor;

    if (should_shoot) {
        target_x = goal_x;
        target_y = (self->position.y > CENTER_Y) ? CENTER_Y - 85.0f : CENTER_Y + 85.0f;
        power_factor = 1.0f; 
    } else {
        float forward_direction = (self->team == 1) ? 1.0f : -1.0f;
        target_x = self->position.x + (250.0f * forward_direction); 
        target_y = (self->position.y < CENTER_Y) ? self->position.y + 50.0f : self->position.y - 50.0f;
        power_factor = 0.6f;
    }

    float dx = target_x - ball->position.x;
    float dy = target_y - ball->position.y;
    float dist = hypotf(dx, dy);
    
    if (dist > 0.0f) {
        float power = ((float)self->talents.shooting / 10.0f) * (MAX_BALL_VELOCITY * power_factor);
        ball->velocity.x = (dx / dist) * power;
        ball->velocity.y = (dy / dist) * power;
    }
}

/* Team 1 shooting logic */
void shooting_logic_1_0(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_1_1(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_1_2(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_1_3(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_1_4(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_1_5(struct Player *self, struct Scene *scene) { 
    if (scene->ball->velocity.x == 0.0f && scene->ball->position.x == CENTER_X) {
        scene->ball->velocity.x = -150.0f;
        scene->ball->velocity.y = 80.0f;   
    } else {
        smart_kick(self, scene);
    }
}

/* Team 2 shooting logic */
void shooting_logic_2_0(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_2_1(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_2_2(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_2_3(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_2_4(struct Player *self, struct Scene *scene) { smart_kick(self, scene); }
void shooting_logic_2_5(struct Player *self, struct Scene *scene) { 
    if (scene->ball->velocity.x == 0.0f && scene->ball->position.x == CENTER_X) {
        scene->ball->velocity.x = 150.0f;
        scene->ball->velocity.y = -80.0f; 
    } else {
        smart_kick(self, scene);
    }
}

//////////////////////////////////////////////////////////////////////////////////

void change_state_prossessor(struct Player *self, struct Scene *scene) {
    struct Ball *ball = scene->ball;
    
    if (ball->possessor == self) {
        self->state = SHOOTING;
        return;
    }
    
    float dx = ball->position.x - self->position.x;
    float dy = ball->position.y - self->position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    
    if (ball->possessor == NULL && dist < 40.0f) {
        self->state = INTERCEPTING;
        return;
    }
    
    self->state = MOVING;
}


/* Team 1 change_state logic */
void change_state_logic_1_0(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_1_1(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_1_2(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_1_3(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_1_4(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_1_5(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }

/* Team 2 change_state logic */
void change_state_logic_2_0(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_2_1(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_2_2(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_2_3(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_2_4(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }
void change_state_logic_2_5(struct Player *self, struct Scene *scene) {change_state_prossessor(self , scene); }

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
    {8, 8, 2, 2},
    {5, 5, 5, 5},
    {7, 6, 3, 4},
    {5, 5, 5, 5},
    {7, 6, 3, 4},
    {4, 4, 8, 4},
};

/* Team 2 */
static struct Talents team2_talents[6] = {
    {7, 6, 3, 4},
    {4, 4, 8, 4},
    {7, 6, 3, 4},
    {8, 8, 2, 2},
    {5, 5, 5, 5},
    {5, 5, 5, 5},
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
    {300, CENTER_Y},
    {250, CENTER_Y-150},
    {200, CENTER_Y-75},
    {150, CENTER_Y},
    {200, CENTER_Y+75},
    {250, CENTER_Y+150},
};

/* Team 2 */
static struct Vec2 team2_positions[6] = {
    {750, CENTER_Y},
    {800, CENTER_Y-150},
    {850, CENTER_Y-75},
    {900, CENTER_Y},
    {850, CENTER_Y+75},
    {800, CENTER_Y+150},
};

struct Vec2 get_positions(int team, int kit) {
    return (team == 1) ? team1_positions[kit] : team2_positions[kit];
}