/*
 * actorA.c
 *
 *  Created on: Nov 17, 2022
 *      Author: worker
 */

#include "main.h"
#include "qpc.h"
#include "actorA.h"
#include "bsp.h"

#include "util.h"

//Q_DEFINE_THIS_FILE

QEvt * ev;
int cnt;

/*..........................................................................*/
typedef struct {     /* the active object */
    QActive super;   /* inherit QActive */

    QTimeEvt timeEvt; /* private time event generator */
    QTimeEvt timeEvt1; /* private time event generator */

    int time_sec;
    int cnt;
} actorA_t;

static actorA_t l_actorA; /* the active object */

QActive * const AO_actorA = &l_actorA.super;

QEvt * ev;
/* hierarchical state machine ... */
static QState actorA_initial(actorA_t * const me, QEvt const * const e);
static QState actorA_S00(actorA_t * const me, QEvt const * const e);

void actorA_ctor(void) {
	actorA_t * const me = &l_actorA;
	QActive_ctor(&me->super, Q_STATE_CAST(&actorA_initial));
	QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
	QTimeEvt_ctorX(&me->timeEvt1, &me->super, TIMEOUT_SIG1, 0U);
}

QState actorA_initial(actorA_t * const me, QEvt const * const e) {
    (void)e; /* avoid compiler warning about unused parameter */

    /* arm the time event to expire in half a second and every half second */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);
    QTimeEvt_armX(&me->timeEvt1, BSP_TICKS_PER_SEC/500U, BSP_TICKS_PER_SEC/500U);
    me->time_sec = 0;
    me->cnt = 0;
    return Q_TRAN(&actorA_S00);
}

QState actorA_S00(actorA_t * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
        	Digit_Number(me->cnt);
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
        	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

        	/*
        	me->time_sec++;
        	if (me->time_sec % 2 ) {
        		Buzz_On();
        	} else {
        		Buzz_Off();
        	}
        	*/

        	// MPU MPU-9250 addr 1101000 = 68 hex
        	// read X accel
        	uint8_t tx_data[8];
        	uint8_t rx_data[8];
        	tx_data[0] = '\x3b';

        	HAL_I2C_Master_Transmit(&hi2c2, 0x68 << 1, tx_data , 1, 10);
        	HAL_I2C_Master_Receive(&hi2c2, 0x68 << 1, rx_data , 1, 10);

        	Digit_Number(rx_data[0]);
        	// TBD : value 0...255 printed on digits 0 ... 99 !

            status = Q_HANDLED();
            break;
        }

        case TIMEOUT_SIG1: {
        	uint8_t rot = Rot_Read();

        	static uint8_t btn_prev, btn;

        	btn = HAL_GPIO_ReadPin(ROTB_GPIO_Port, ROTB_Pin);

        	if (btn ^ btn_prev) {
        	    ev =  Q_NEW(QEvt, ROT_BTN_SIG);
        	    QActive_post_((QActive *)me, ev, QF_NO_MARGIN, NULL);
        	}

        	btn_prev = btn;

        	if (rot == 2) {
        	    ev =  Q_NEW(QEvt, ROT_UP_SIG);
        	    QActive_post_((QActive *)me, ev, QF_NO_MARGIN, NULL);
        	} else if (rot == 3) {
        	    ev =  Q_NEW(QEvt, ROT_DN_SIG);
        	    QActive_post_((QActive *)me, ev, QF_NO_MARGIN, NULL);
        	}

            status = Q_HANDLED();
            break;
        }

        case ROT_BTN_SIG: {
        	static uint8_t x = 0;
        	x++;
        	if (x % 2) {
        		Buzz_On();
        	} else {
        		Buzz_Off();
        	}
            status = Q_HANDLED();
            break;
        }

        case ROT_UP_SIG: {
        	if (me->cnt < 99) {
        		me->cnt++;
        		Digit_Number(me->cnt);
        	}
            status = Q_HANDLED();
            break;
        }

        case ROT_DN_SIG: {
        	if (me->cnt > 0) {
        		me->cnt--;
        		Digit_Number(me->cnt);
        	}
            status = Q_HANDLED();
            break;
        }

        default: {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}



