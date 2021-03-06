#include "ai_common.h"

#define CURLY_STAND				0
#define CURLY_WALK				3
#define CURLY_WALKING			4

void onspawn_curly_collapsed(Entity *e) {
	e->state = 32;
	e->y += 8 << CSF;
}

void ai_curly(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	switch(e->state) {
		case 0:							// state 0: stand and do nothing
		{
			e->frame = 0;
			e->eflags |= NPC_INTERACTIVE;	// needed for after Almond battle
		}
		/* no break */
		case 1:
		{
			e->x_speed = 0;
			// important that state 1 does not change look-away frame for Drain cutscene
			if(e->frame == 9) e->state = 32;
			else e->frame = 0;
		}
		break;
		case 3:							// state 3: walk forward
		case 10:						// state 10: walk to player and stop
		{
			if (e->state == 10) FACE_PLAYER(e);
			e->state++;
		}
		/* no break */
		case 4:
		case 11:
		{
			ANIMATE(e, 8, 1,0,2,0);
			if (e->state == 11 && PLAYER_DIST_X(20<<CSF)) {
				e->state = 0;
				break;
			}
			//if (!e->grounded) e->frame = 5;
			MOVE_X(SPEED(0x200));
		}
		break;
		// state 5: curly makes a "kaboom", then looks sad.
		case 5:
		{
			e->state = 6;
			//SmokeClouds(e, 8, 0, 0);
		}
		/* no break */
		case 6:
		{
			e->frame = 10;
		}
		break;
		case 20:			// face away
		{
			e->x_speed = 0;
			e->frame = 6;
		}
		break;
		case 21:			// look up
		{
			e->x_speed = 0;
			e->frame = 2;
		}
		break;
		case 30:			// state 30: curly goes flying through the air and is knocked out
		{
			e->state = 31;
			e->frame = 10;
			e->timer2 = 0;
			e->y_speed = SPEED(-0x400);
			e->grounded = FALSE;
			MOVE_X(SPEED(-0x200));
		}
		/* no break */
		case 31:
		{
			if (e->grounded) e->state = 32;
			else break;
		}
		/* no break */
		case 32:			// state 32: curly is laying knocked out
		{
			e->frame = 9;
			e->x_speed = 0;
		}
		break;
		
		// walk backwards from collapsing wall during final cutscene
		case 70:
		{
			e->state = 71;
			e->timer = 0;
			e->frame = 1;
		}
		/* no break */
		case 71:
		{
			MOVE_X(SPEED(-0x100));
		}
		break;
	}
	e->y = e->y_next;
	e->x = e->x_next;
	if(!e->grounded && e->state != 32) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
}

// curly being carried by Tow Rope
void ai_curly_carried(Entity *e) {
	// Keep in front of doors
	if(abs(e->x_mark - camera.x) > SCREEN_HALF_W || abs(e->y_mark - camera.y) > SCREEN_HALF_H) {
		moveMeToFront = TRUE;
		e->x_mark = camera.x;
		e->y_mark = camera.y;
	}
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->frame = 10;
			e->eflags &= ~NPC_INTERACTIVE;
			e->nflags &= ~NPC_INTERACTIVE;
		}
		/* no break */
		case 1:
		{	// carried by player
			e->dir = player.dir;
			e->x = player.x + pixel_to_sub(e->dir ? -4 : 4);
			e->y = player.y - pixel_to_sub(5);
		}
		break;
		// floating away after Ironhead battle
		case 10:
		{
			e->x_speed = SPEED(0x40);
			e->y_speed = SPEED(-0x20);
			e->state = 11;
		}
		/* no break */
		case 11:
		{
			if (e->y < block_to_sub(4))	{
				// if in top part of screen, reverse before hitting wall
				e->y_speed = SPEED(0x20);
			}
		}
		break;
		case 20:
		{
			e->state = STATE_DELETE;
		}
		break;
	}
	
}

#define CURLYB_FIGHT_START		10
#define CURLYB_WAIT				11
#define CURLYB_WALK_PLAYER		13
#define CURLYB_WALKING_PLAYER	14
#define CURLYB_CHARGE_GUN		20
#define CURLYB_FIRE_GUN			21
#define CURLYB_SHIELD			30

static void curlyboss_fire(Entity *e, uint8_t dir) {
	Entity *shot = entity_create(e->x, e->y, OBJ_CURLYBOSS_SHOT, 0);
	shot->dir = dir & 1;
	shot->attack = 6;
	switch(dir) {
		case 0:
			shot->x = e->x - pixel_to_sub(12);
			shot->y = e->y + pixel_to_sub(2);
			shot->x_speed = -4096;
		break;
		case 1:
			shot->x = e->x + pixel_to_sub(12);
			shot->y = e->y + pixel_to_sub(2);
			shot->x_speed = 4096;
		break;
		case 2:
			shot->x = e->x;
			shot->y = e->y - pixel_to_sub(10);
			shot->y_speed = -4096;
			shot->frame = 2;
		break;
	}
	sound_play(SND_POLAR_STAR_L1_2, 4);
}

void ai_curlyBoss(Entity *e) {
	switch(e->state) {
		case CURLYB_FIGHT_START:
		{
			e->state = CURLYB_WAIT;
			e->timer = (random() % 50) + 50;
			e->frame = 0;
			e->dir = (e->x <= player.x);
			e->eflags |= NPC_SHOOTABLE;
			e->eflags &= ~NPC_INVINCIBLE;
			e->nflags &= ~NPC_INVINCIBLE;
		}
		/* no break */
		case CURLYB_WAIT:
		{
			if(!e->timer--) e->state = CURLYB_WALK_PLAYER;
		}
		break;
		case CURLYB_WALK_PLAYER:
		{
			e->state = CURLYB_WALKING_PLAYER;
			e->frame = 1;
			e->timer = (random() % 50) + 50;
			FACE_PLAYER(e);
		}
		/* no break */
		case CURLYB_WALKING_PLAYER:
		{
			ANIMATE(e, 8, 1,0,2,0);
			ACCEL_X(SPEED(0x40));
			if (e->timer) {
				e->timer--;
			} else {
				e->eflags |= NPC_SHOOTABLE;
				e->state = CURLYB_CHARGE_GUN;
				e->timer = 0;
				sound_play(SND_CHARGE_GUN, 5);
			}
		}
		break;
		case CURLYB_CHARGE_GUN:
		{
			FACE_PLAYER(e);
			e->x_speed -= e->x_speed >> 3;
			e->frame = 0;
			if (++e->timer > TIME(50)) {
				e->state = CURLYB_FIRE_GUN;
				e->x_speed = 0;
				e->timer = 0;
			}
		}
		break;
		case CURLYB_FIRE_GUN:
		{
			if (e->timer % TIME(5) == 0) {	// time to fire
				// check if player is trying to jump over
				if (abs(e->x - player.x) < pixel_to_sub(32) && player.y + pixel_to_sub(10) < e->y) {
					// shoot up instead
					e->frame = 3;
					curlyboss_fire(e, 2);
				} else {
					e->frame = 0;
					curlyboss_fire(e, e->dir);
				}
			}
			if (++e->timer > TIME(30)) e->state = CURLYB_FIGHT_START;
		}
		break;
		case CURLYB_SHIELD:
		{
			e->x_speed = 0;
			if (++e->timer > TIME(30)) e->state = CURLYB_FIGHT_START;
		}
		break;
	}

	if (e->state > CURLYB_FIGHT_START && e->state < CURLYB_SHIELD) {
		// curly activates her shield anytime a missile's explosion goes off,
		// even if it's nowhere near her at all
		if(bullet_missile_is_exploding()) {
			e->timer = 0;
			e->state = CURLYB_SHIELD;
			e->frame = 4;
			e->eflags &= ~NPC_SHOOTABLE;
			e->eflags |= NPC_INVINCIBLE;
			e->x_speed = 0;
		}
	}
	
	if (e->x_speed > SPEED(0x200)) e->x_speed = SPEED(0x200);
	if (e->x_speed < -SPEED(0x200)) e->x_speed = -SPEED(0x200);

	e->x_next = e->x + e->x_speed;
	e->y_next = e->y;

	collide_stage_leftwall(e);
	collide_stage_rightwall(e);

	e->x = e->x_next;
}

void ondeath_curlyBoss(Entity *e) {
	//if(e->state == STATE_DEFEATED) {
		////SPR_SAFERELEASE(e->sprite);
		entity_default(e, OBJ_CURLY, 0);
		//entity_sprite_create(e);
		e->x -= 8;
		e->eflags &= ~NPC_SHOOTABLE;
		e->state = 0;
		entities_clear_by_type(OBJ_CURLYBOSS_SHOT);
		tsc_call_event(e->event);
	//}
}

void ai_curlyBossShot(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(collide_stage_leftwall(e) || collide_stage_rightwall(e) || collide_stage_ceiling(e)) {
		e->state = STATE_DELETE;
	} else if(!player_invincible() && entity_overlapping(e, &player)) {
		player_inflict_damage(e->attack);
		e->state = STATE_DELETE;
	} else {
		e->x = e->x_next;
		e->y = e->y_next;
	}
}
