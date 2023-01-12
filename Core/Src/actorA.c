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
    int time_sec;
    int cnt;
} actorA_t;

static actorA_t l_actorA; /* the active object */

QActive * const AO_actorA = &l_actorA.super;

/* hierarchical state machine ... */
static QState actorA_initial(actorA_t * const me, QEvt const * const e);
static QState actorA_S00(actorA_t * const me, QEvt const * const e);

void actorA_ctor(void) {
	actorA_t * const me = &l_actorA;
	QActive_ctor(&me->super, Q_STATE_CAST(&actorA_initial));
	QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
}

QState actorA_initial(actorA_t * const me, QEvt const * const e) {
    (void)e; /* avoid compiler warning about unused parameter */

    /* arm the time event to expire in half a second and every half second */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);
    me->time_sec = 0;
    return Q_TRAN(&actorA_S00);
}

QState actorA_S00(actorA_t * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {

            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
        	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        	Digit_Number(me->time_sec++);

        	if (me->time_sec % 2 ) {
        		Buzz_On();
        	} else {
        		Buzz_Off();
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



