
#include <xcopy.h>

volatile int        tc_alarm_update_time;

volatile char      *tc_error_log_time;
volatile time_t     tc_current_time_sec;
volatile long       tc_current_time_msec;
volatile struct tm  tc_current_tm;

static char cache_err_log_time[TC_ERR_LOG_TIME_LEN];

int
tc_time_init(long msec)
{
    struct itimerval itv;

    tc_alarm_update_time = 0;

    itv.it_value.tv_sec  = msec / 1000;
    itv.it_value.tv_usec = (msec % 1000) * 1000;
    itv.it_interval.tv_sec = msec / 1000;
    itv.it_interval.tv_usec = (msec % 1000) * 1000;

    if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
        tc_log_info(LOG_ERR, errno, "setitimer failed");
        return TC_ERROR;
    }

    tc_time_update();

    return TC_OK;
}

void
tc_time_update()
{
    long            msec, cur_time_msec;
    time_t          sec;
    struct tm       tm;
    struct timeval  tv;

    gettimeofday(&tv, NULL);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;

    tc_current_time_sec = sec;
    cur_time_msec = sec * 1000 + msec;

    if (cur_time_msec != tc_current_time_msec) {

        tc_current_time_msec = cur_time_msec;

        tc_localtime(sec, &tm);

        sprintf(cache_err_log_time, "%4d/%02d/%02d %02d:%02d:%02d +%03d",
                tm.tm_year, tm.tm_mon,
                tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec,
                (int) msec);

        tc_current_tm = tm;
        tc_error_log_time = cache_err_log_time;
    }
}

void
tc_localtime(time_t sec, struct tm *tm)
{
#if (HAVE_LOCALTIME_R)
    (void) localtime_r(&sec, tm);
#else
    struct tm *t;

    t = localtime(&sec);
    *tm = *t;
#endif

    tm->tm_mon++;
    tm->tm_year += 1900;
}
