#include "g_local.h"

edict_t* FindClosestPlayer(edict_t* self);
void RunMonsterTurnAI(edict_t* ent);
turn_state_t current_turn = TURN_PLAYER;
edict_t* currentActiveUnit = NULL;
int turn_timer = 0;

// Optional helper for debugging
void PrintTurnState(void) {
    if (current_turn == TURN_PLAYER)
        gi.dprintf("[TURN] Player's turn\n");
    else
        gi.dprintf("[TURN] Enemy's turn\n");
}

// Called every frame to update logic for player-controlled units
void TurnThink_Player(edict_t* ent) {
    if (!ent->inuse || ent->team != TEAM_PLAYER)
        return;

    if (current_turn != TURN_PLAYER)
        return;

    if (ent->hasActed)
        return;

    if (!ent->hasActed) {
        ent->actionsLeft = 2;  // or 1, depending on balance
        VectorCopy(ent->s.origin, ent->lastPosition);  // track for movement
    }
    gi.dprintf("[PLAYER] Waiting for player input on unit %d\n", ent->s.number);

    // TODO: Add input logic or queued action system
    ent->nextthink = 0;
}

// Called during the enemy turn to run AI logic
void TurnThink_Enemy(edict_t* ent) {
    if (!ent->inuse || ent->health <= 0 || ent->modTeam != TEAM_ENEMY || current_turn != TEAM_ENEMY || ent->hasActed)
        return;

    if (!ent->enemy || !ent->enemy->inuse) {
        ent->enemy = FindClosestPlayer(ent);
    }

    ent->aiTurnFrames = 0;
    ent->think = RunMonsterTurnAI;
    ent->nextthink = level.time + FRAMETIME;
}



// Checks if all units of a team have acted
qboolean AllUnitsActed(int team) {
    edict_t* ent;
    for (int i = 0; i < globals.num_edicts; i++) {
        ent = &g_edicts[i];
        if (!ent->inuse) continue;
        if (ent->modTeam == team && !ent->hasActed)
            return false;
    }
    return true;
}

// Resets all units of a team for the next turn
void ResetUnitActions(int team) {
    edict_t* ent;
    for (int i = 0; i < globals.num_edicts; i++) {
        ent = &g_edicts[i];
        if (!ent->inuse) continue;
        if (ent->modTeam == team) {
            ent->hasActed = false;
            ent->hasMoved = false;
            ent->hasShot = false;
        }
    }
}


void UpdateTurnManager(void) {
    if (AllUnitsActed(current_turn)) {
        gi.dprintf("[TURN] Ending turn for team %d\n", current_turn);
        ResetUnitActions(current_turn);

        // Switch to the next team
        current_turn = (current_turn == TEAM_PLAYER) ? TEAM_ENEMY : TEAM_PLAYER;
        PrintTurnState();

        // Reset active unit and assign new one for the next team
        currentActiveUnit = NULL;
        TurnManagerThink();  // <- this triggers the next AI or player unit to start thinking
    }
}


void TurnManagerThink(void) {
    if (!currentActiveUnit) {
        gi.dprintf("[DEBUG] Scanning for next active unit...\n");

        for (int i = 0; i < globals.num_edicts; i++) {
            edict_t* ent = &g_edicts[i];
            gi.dprintf("[DEBUG] ent %d - inuse=%d modTeam=%d hasActed=%d classname=%s\n",
                i, ent->inuse, ent->modTeam, ent->hasActed, ent->classname ? ent->classname : "null");
        }

        // Try to find the next unit from current_turn's team
        for (int i = 0; i < globals.num_edicts; i++) {
            edict_t* ent = &g_edicts[i];
            if (!ent->inuse || ent->modTeam != current_turn || ent->hasActed)
                continue;

            currentActiveUnit = ent;
            gi.dprintf("[TURN] New active unit: %d\n", ent->s.number);

            // Schedule their logic instead of running it immediately
            if (current_turn == TEAM_PLAYER) {
                TurnThink_Player(ent);  // sets nextthink = 0
            }
            else {
                TurnThink_Enemy(ent);   // sets think = RunMonsterTurnAI and schedules it
            }

            return;  // stop here — one unit per frame
        }

        // No units left — switch teams
        gi.dprintf("[TURN] All units acted. Switching turns...\n");

        current_turn = (current_turn == TEAM_PLAYER) ? TEAM_ENEMY : TEAM_PLAYER;
        PrintTurnState();

        currentActiveUnit = NULL;
    }
}




float VectorDistance(vec3_t a, vec3_t b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

qboolean IsUnitReady(edict_t* ent) {
    return ent->inuse && ent->modTeam == current_turn && !ent->hasActed;
}

void EndUnitTurn(edict_t* ent) {
    ent->hasActed = true;

    if (currentActiveUnit == ent) {
        currentActiveUnit = NULL;
        gi.dprintf("[DEBUG] Turn ended for unit %d, clearing active unit\n", ent->s.number);
    }
    else {
        gi.dprintf("[DEBUG] Tried to end unit %d, but it wasn't the current active unit\n", ent->s.number);
    }
    UpdateTurnManager();
}

void RunMonsterTurnAI(edict_t* ent) {
    if (!ent || !ent->inuse || ent->health <= 0 || current_turn != TEAM_ENEMY)
        return;

    if (ent->aiTurnFrames == 0) {
        gi.dprintf("[AI TURN] Unit %d starting turn\n", ent->s.number);

        // Always reassign enemy
        edict_t* newTarget = FindClosestPlayer(ent);
        if (newTarget && newTarget->inuse) {
            ent->enemy = newTarget;
            FoundTarget(ent);  // Reset combat state
        }

        // Force them into a combat-ready state every turn
        ent->monsterinfo.aiflags &= ~AI_STAND_GROUND;
        ent->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
        ent->monsterinfo.pausetime = 0;
        ent->monsterinfo.attack_state = AS_STRAIGHT;

        // Always set currentmove to run
        if (ent->monsterinfo.run)
            ent->monsterinfo.run(ent);
    }

    // Run FSM logic
    M_MoveFrame(ent);

    ent->aiTurnFrames++;

    if (ent->aiTurnFrames >= 25) {
        gi.dprintf("[AI TURN] Unit %d ending turn\n", ent->s.number);
        ent->aiTurnFrames = 0;

        // top further processing after turn ends
        ent->think = NULL;
        ent->nextthink = 0;

        EndUnitTurn(ent);
    }
    else {
        ent->nextthink = level.time + FRAMETIME;
        ent->think = RunMonsterTurnAI;
    }
}



edict_t* FindClosestPlayer(edict_t* self) {
    edict_t* closest = NULL;
    float closestDistSq = 99999999.0f;

    for (int i = 0; i < globals.num_edicts; i++) {
        edict_t* other = &g_edicts[i];
        if (!other->inuse || other->modTeam != TEAM_PLAYER || other->health <= 0)
            continue;

        // Optional: only pick visible enemies
        //if (!visible(self, other))
        //    continue;

        float dx = other->s.origin[0] - self->s.origin[0];
        float dy = other->s.origin[1] - self->s.origin[1];
        float dz = other->s.origin[2] - self->s.origin[2];
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closest = other;
        }
    }

    return closest;
}

