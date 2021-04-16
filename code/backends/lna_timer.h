#ifndef LNA_BACKENDS_LNA_TIMER_BACKEND_H
#define LNA_BACKENDS_LNA_TIMER_BACKEND_H

typedef struct lna_timer_s lna_timer_t;

typedef struct lna_second_s         { double value; } lna_second_t;
typedef struct lna_millisecond_s    { double value; } lna_millisecond_t;

extern void                 lna_timer_start             (lna_timer_t* timer);
extern void                 lna_timer_update            (lna_timer_t* timer);
extern lna_second_t         lna_timer_elasped_time_in_s (lna_timer_t* timer);
extern lna_millisecond_t    lna_timer_elasped_time_in_ms(lna_timer_t* timer);
extern lna_second_t         lna_timer_delta_time_in_s   (lna_timer_t* timer);
extern lna_millisecond_t    lna_timer_delta_time_in_ms  (lna_timer_t* timer);

#endif
