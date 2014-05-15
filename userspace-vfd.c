/*
 * Utility for managing VFD (Versatile Front Panel Display) on some Shuttle
 * models. Requires libusb.
 *
 * - Icons: clock, radio, music, CD/DVD, television, camera, rewind, record,
 *   play, pause, stop, fast-forward, reverse, repeat, muteé, 12 volume bars.
 *
 * Based on shuttleVFD.c (lcdproc) and setvfd (by Jeremy James)
 * Creation Date: 12.09.2008 19:54
 *

TODO:
- vfd_init should not be called when user request --help or --version.
- blocking orders:
  - cpu load (1 or 2 cpus)
  - cpu temp / fans / sensors
  - mplayer (via lirc interface?)
- system monitoring non blocking orders:
  - icons: auto from snd card master channel

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>
#include <signal.h>
#include <sys/sysinfo.h>

#include "shuttle_vfd.h"
#include "handler_list.h"


/* some defines */
#define PROGRAM_NAME      "userspace-vfd"
#define PROGRAM_VERSION   "1.0"

#define TEST_STRING     "### Hello  World ###"
#define BUFFER_SZ       252

const char *spaces = "                    ";
const useconds_t attente = 400000; // 0.5s

/* global variables */
static char vfd_buf[SHUTTLE_VFD_WIDTH];
static handler_list_t vfd_orders;
static volatile int quit = 0;

static char buffer[BUFFER_SZ+4];

/* local prototypes */
static int app_display_text(const char *);
static int app_display_centered_text(const char *);


/* Functions definition */

/* ------------------------------------------------------------------------- */

/* Left aligned text. If text is too large, it's truncated */
static int app_display_text(const char *text)
{
  int len, i = 0;

  len = strlen(text);
  if (len > SHUTTLE_VFD_WIDTH) {
    fprintf(stderr, "wrn: truncating text\n");
  } else {
    memset(vfd_buf, (int)' ', SHUTTLE_VFD_WIDTH);
  }

  vfd_clear(1);

  while ((i<SHUTTLE_VFD_WIDTH) && (text[i] != 0)) {
    vfd_buf[i] = text[i];
    i++;
  }

  return vfd_display_text(vfd_buf, SHUTTLE_VFD_WIDTH, 0);
}


/* If text is too large, it's truncated */
static int app_display_centered_text(const char *text)
{
  int len = strlen(text);
  if (len > SHUTTLE_VFD_WIDTH) {
    fprintf(stderr, "wrn: truncating text\n");
  } else {
    memset(vfd_buf, (int)' ', SHUTTLE_VFD_WIDTH);
  }

  vfd_clear(1);
  memcpy(vfd_buf + (SHUTTLE_VFD_WIDTH - len)/2, text, len);

  return vfd_display_text(vfd_buf, SHUTTLE_VFD_WIDTH, 0);
}

/* ------------------------------------------------------------------------- */

static int cb_date_and_time(void *param)
{
  handler_clock_t *h = (handler_clock_t *)param;

  struct tm *now;
  time_t t;

  if (h->format == NULL)
    h->format = "%X (%a %d)"; // "%H:%M:%S";

  time(&t);
  now = localtime(&t);
  strftime (buffer, BUFFER_SZ, h->format, now);

  // display text without scrolling
  app_display_centered_text(buffer);

  return 0;
}

/* ------------------------------------------------------------------------- */

static int cb_text(void *param)
{
  handler_text_t *h = (handler_text_t *)param;

  int i, len;

  len = strlen(h->message);
  if ((len + 2*SHUTTLE_VFD_WIDTH) > 100) {
    len = 100 - 2*SHUTTLE_VFD_WIDTH;
    fprintf(stderr, "wrn: truncating text (%d)\n", len);
  }

  /* Build the big one-line message */
  memcpy(buffer, spaces, SHUTTLE_VFD_WIDTH);
  memcpy(buffer + SHUTTLE_VFD_WIDTH, h->message, len);
  memcpy(buffer + SHUTTLE_VFD_WIDTH + len, spaces, SHUTTLE_VFD_WIDTH);

  // Display per page
  if (h->style & 0x1)
  {
    for (i = 1; i <= ((len/SHUTTLE_VFD_WIDTH)+1) && !quit; i++) {
      vfd_display_text(buffer + i*SHUTTLE_VFD_WIDTH, SHUTTLE_VFD_WIDTH,
          5*attente);
    }
  }
  // Scrolling display
  else
  {
    for (i = 0; i <= (len + SHUTTLE_VFD_WIDTH) && !quit; i++) {
      vfd_display_text(buffer + i, SHUTTLE_VFD_WIDTH, attente);
    }
  }

  return 0;
}


static int cb_text_uptime(void *param)
{
  //handler_text_t *h = (handler_text_t *)param;
  struct sysinfo info;
  int len, i;

  sysinfo(&info);

  buffer[0] = 0;
  len = snprintf(buffer, BUFFER_SZ, "%sUptime: %02ld:%02ld%s",
      spaces, info.uptime/3600, (info.uptime%3600)/60, spaces);

  /* The big one-line message is ready, let's display it */
  for (i = 0; i <= (len - SHUTTLE_VFD_WIDTH)  && !quit; i++) {
    vfd_display_text(buffer + i, SHUTTLE_VFD_WIDTH, attente);
  }

  return 0;
}


/* ------------------------------------------------------------------------- */


static void sig_int(int signo)
{
  signal(signo, SIG_DFL);
  quit = 1;
  fprintf(stderr, "quitting...\n");
}


static void version(void)
{
  fprintf(stdout, "%s %s\n"
      "Copyright © 2008 Free Software Foundation, Inc.\n"
      "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
      "This is free software: you are free to change and redistribute it.\n"
      "There is NO WARRANTY, to the extent permitted by law.\n"
      "\n"
      "Written by Matthieu Crapet <mcrapet@gmail.com>.\n",
      PROGRAM_NAME, PROGRAM_VERSION);
}


static void help(void)
{
  fprintf(stdout, "Usage: %s [OPTIONS]\n"
      "Shuttle SG33G5M VFD manager.\n"
      "\n"
      "Non blocking orders:\n"
      "   -m, --message=STRING  Display message (truncatedi if > %d chars)\n"
      "   -i, --icons=LIST      Display icon(s). List is comma separated. Values:\n"
      "                         all,clk,rad,mus,cd,tv,cam,rew,rec,play,pause,stop,ff,rev,rep,mute.\n"
      "                         For volume: choose one of: vol0,vol1,vol2,...,vol11,vol12.\n"
      "                         Without argument, clear icons.\n"
      "       --vol=PERCENT     Manual volume control (value from 0 to 100).\n"
      "   -c, --clean           Clean display and icons\n"
      "       --clock           Build-in Cypress feature\n"
      "       --test            Display all icons and fill with text\n"
      "\n"
      "Blocking orders:\n"
      "   -t, --time            Display date/time\n"
      "       --msg=STRING      Display message (circular scrolling)\n"
      "       --msg2=STRING     Display message (per page)\n"
      "       --msg_uptime      Display system infos\n"
      "\n"
      "Misc options:\n"
      "  -h,  --help            display this help and exit\n"
      "       --version         display program version and exit\n",
      PROGRAM_NAME, SHUTTLE_VFD_WIDTH);
}


/* Better than res = atoi(p) */
static int parse_number(const char *p, int *res)
{
  char *endptr;

  if ((p[0] == '0') && (p[1] == 'x')) {
    *res = strtoul(p, &endptr, 16);
  } else {
    *res = strtoul(p, &endptr, 10);
  }

  if (*endptr != '\0') /* le parsing n'a pas pris entièrement le buffer */
    return -1;
  return 0;
}


static unsigned long parse_icons(const char *text)
{
  char delims[] = ",";
  char *result = NULL;
  unsigned long i, ret = 0;

  result = strtok((char *)text, delims);
  while(result != NULL ) {

    if (vfd_parse_icons(result, &i) == 0)
      ret |= i;
    else
      fprintf(stderr, "wrn: unknown icon %s, ignoring\n", result);

    result = strtok(NULL, delims);
  }
  return ret;
}


int main(int argc, char *argv[])
{
  int c, ret;
  int option_index = 0;  /* getopt_long stores the option index here. */

  static char short_options[] = "hcm:i:t";
  static struct option long_options[] = {
    {"message", required_argument, NULL, 'm'},
    {"icons",   optional_argument, NULL, 'i'},
    {"clean",   no_argument, 0, 'c' },
    {"test",    no_argument, 0, 'e' },
    {"vol",     required_argument, 0, 'o'},
    {"msg",     required_argument, 0, 'n'},
    {"msg2",    required_argument, 0, 'q'},
    {"msg_uptime", no_argument, 0, 'p' },
    {"clock",   no_argument, 0, 'b' },
    {"time",    no_argument, 0, 't' },
    {"version", no_argument, 0, 'v' },
    {"help",    no_argument, 0, 'h' },
    {0, 0, 0, 0}
  };

  /* set FR locale. FIXME! */
  setlocale(LC_TIME, "fr_FR.UTF-8");

  handler_init(&vfd_orders);

  ret = vfd_init(SHUTTLE_VFD_VENDOR_ID, SHUTTLE_VFD_PRODUCT_ID,
      SHUTTLE_VFD_INTERFACE_NUM);

  if (ret == 0) {
    handler_t req;

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
      switch(c) {

        /* Generic options */
        case 'h':
          help();
          return 0;
        case 'v':
          version();
          return 0;
        case '?':
          fprintf(stderr, "%s: bad option (%c)\nTry `%s --help' for more information.\n",
              PROGRAM_NAME, optopt, PROGRAM_NAME);
          return -1;

        /* Non blocking requests */
        case 'c':
          vfd_clear(0);
          break;
        case 'm':
          app_display_text(optarg);
          break;
        case 'i':
          if (optarg == NULL)
            vfd_display_icons(0);
          else
            vfd_display_icons(parse_icons(optarg));
          break;
        case 'e':
          app_display_text(TEST_STRING);
          vfd_display_icons(SHUTTLE_VFD_ALL_ICONS);
          break;
        case 'o':
          parse_number(optarg, &ret);
          if (ret == 0)
            vfd_display_icons(SHUTTLE_VFD_ICON_MUTE);
          else {
            if (ret > 100) ret = 100;
            vfd_display_icons((3*ret/25+1) << 15);
          }
          break;
        case 'b':
          vfd_display_clock();
          break;

        /* Blocking requests */
        case 't':
          req.command = ORDER_HANDLER_CLOCK;
          req.cb = cb_date_and_time;
          req.data.clock.format = NULL;

          if (handler_add(&vfd_orders, &req) == NULL)
            fprintf(stderr, "err: can't add handler\n");
          break;

        case 'n':
          req.command = ORDER_HANDLER_MESSAGE;
          req.cb = cb_text;
          req.data.text.message = optarg;
          req.data.text.style = 0;

          if (handler_add(&vfd_orders, &req) == NULL)
            fprintf(stderr, "err: can't add handler\n");
          break;

        case 'q':
          req.command = ORDER_HANDLER_MESSAGE;
          req.cb = cb_text;
          req.data.text.message = optarg;
          req.data.text.style = 1;

          if (handler_add(&vfd_orders, &req) == NULL)
            fprintf(stderr, "err: can't add handler\n");
          break;

        case 'p':
          req.command = ORDER_HANDLER_MESSAGE_UPTIME;
          req.cb = cb_text_uptime;
          req.data.text.message = NULL;
          req.data.text.style = 0;

          if (handler_add(&vfd_orders, &req) == NULL)
            fprintf(stderr, "err: can't add handler\n");
          break;

      }
    } //while

    /* If we have blocking requests, treat them */
    if (handler_count(&vfd_orders) > 0) {
      int i;
      handler_t *pReq;

      fprintf(stderr, "dbg: processing orders\n");

      /* setup signal handler for quitting */
      signal(SIGINT,  sig_int);
      signal(SIGTERM, sig_int);
      signal(SIGQUIT, sig_int);

      while (!quit) {

        for (i = 0; i < handler_count(&vfd_orders); i++) {
          pReq = handler_get(&vfd_orders, i);

          switch (pReq->command) {
            case ORDER_HANDLER_CLOCK:
            case ORDER_HANDLER_MESSAGE:
            case ORDER_HANDLER_MESSAGE_UPTIME:
              pReq->cb(&req.data);
              break;

            default:
              fprintf(stderr, "err: unknow order handler\n");
          }

        } // foreach handler
        usleep(attente);

      }
    }

    vfd_close(SHUTTLE_VFD_INTERFACE_NUM);
  }

  return 0;
}
