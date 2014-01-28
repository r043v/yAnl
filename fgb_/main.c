
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strdup(char *s);

#include <stdarg.h>
#include <signal.h>

#include "defs.h"
#include "input.h"
#include "rc.h"

int strncmp (const char *ch1, const char *ch2, size_t l);
#define Version "*"

static char *defaultconfig[] =
{	NULL
};

static void banner(){}
static void copyright(){}
static void usage(char *name){}
static void copying(){}
static void help(char *name){}
static void version(char *name){}

void doevents()
{	event_t ev;
	int st;
	ev_poll();
	while (ev_getevent(&ev))
	{
		if (ev.type != EV_PRESS && ev.type != EV_RELEASE)
			continue;
		st = (ev.type != EV_RELEASE);
		rc_dokey(ev.code, st);
	}
}

static void shutdown()
{	vid_close();
	pcm_close();
}

void die(char *fmt, ...){	exit(1);}

static char *base(char *s)
{	char *p;
	p = strrchr(s, '/');
	if (p) return p+1;
	return s;
}

int main(int argc, char *argv[]){	return 0;	}











