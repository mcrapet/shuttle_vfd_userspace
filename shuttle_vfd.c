/*
 * shuttle_vfd.h - Low-level communication layer.
 * Copyright (C) 2008 Matthieu Crapet <mcrapet@gmail.com>
 *
 * Shuttle SG33G5M VFD (20x1 character display. Each character cell is 5x8 pixels)
 * - The display is driven by Princeton Technologies PT6314 VFD controller
 * - Cypress CY7C63723C (receives USB commands and talk to VFD controller)
 *
 * LCD "prococol" : each message has a length of 8 bytes
 * - 1 nibble: command (0x1, 0x3, 0x7, 0x9, 0xD)
 *     - 0x1 : clear text and icons (len=1)
 *     - 0x7 : icons (len=4)
 *     - 0x9 : text (len=7)
 *     - 0xD : set clock data (len=7)
 *     - 0x3 : display clock (internal feature) (len=1)
 * - 1 nibble: message length (0-7)
 * - 7 bytes : message data
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <time.h>

#include "shuttle_vfd.h"

#define DEC_AS_HEX(v)   (((v)/10 * 16) + ((v)%10))

/* Global data */
static usb_dev_handle *vfd_dev;


int vfd_init(int vendor_id, int product_id, int interface)
{
  int ret = 0;
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_init();
  usb_find_busses();
  usb_find_devices();

  for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
    for (dev = bus->devices; dev != NULL; dev = dev->next) {
      if (dev->descriptor.idVendor == vendor_id &&
          dev->descriptor.idProduct == product_id) {
        vfd_dev = usb_open(dev);
      }
    }
  }

  if (vfd_dev == NULL) {
    fprintf(stderr, "err: can't open Shuttle VFD\n");
    ret = -1;
  }

  if (usb_claim_interface(vfd_dev, interface) < 0) {
    usb_close(vfd_dev);

    // TODO check for root user ?
    fprintf(stderr, "err: unable to claim interface. You may retry with root privileges.\n");
    ret = -2;
  }

  return ret;
}


int vfd_close(int interface)
{
  int ret = 0;

  if (usb_release_interface(vfd_dev, SHUTTLE_VFD_INTERFACE_NUM) < 0) {
    fprintf(stderr, "err: unable to release interface\n");
    ret = -1;
  }

  if (usb_close(vfd_dev) < 0) {
    fprintf(stderr, "err: can't close Shuttle VFD\n");
    ret = -2;
  }

  return ret;
}


int vfd_send_packet(unsigned char packet[SHUTTLE_VFD_PACKET_SIZE])
{
  int i, ret = -1;

  for (i = 0; i < SHUTTLE_VFD_WRITE_ATTEMPTS; i++) {
    if (usb_control_msg(vfd_dev,
          0x21,      // requesttype
          0x09,      // request
          0x0200,    // value
          0x0001,    // index
          (char *)packet,
          SHUTTLE_VFD_PACKET_SIZE, 100) == SHUTTLE_VFD_PACKET_SIZE) {

      usleep(SHUTTLE_VFD_SUCCESS_SLEEP_USEC);
      ret = 0;
      break;
    }

    fprintf(stderr, "wrn: write failed retrying...\n");
    usleep(SHUTTLE_VFD_RETRY_SLEEP_USEC);
  }

  return ret;
}


int vfd_clear(int b)
{
  unsigned char packet[SHUTTLE_VFD_PACKET_SIZE];
  memset(packet, 0, SHUTTLE_VFD_PACKET_SIZE);
  packet[0] = (1 << 4) + 1;

  if (b == 0)
    packet[1] = 1; // full clear (text + icons)
  else
    packet[1] = 2; // just reset the text cursor (keep text)

  return vfd_send_packet(packet);
}


/* Built-in feature (of Cypress controller), will display SHUTTLE_VFD_ICON_CLOCK */
int vfd_display_clock(void)
{
  unsigned char packet[SHUTTLE_VFD_PACKET_SIZE];
  struct tm *now;
  time_t t;

  time(&t);
  now = localtime(&t);

  /* Warning: Hexa values are decimal values !
   * 30 16 14 07 14 09 08 : "Sep 14 Sun 02:16 PM"
   */
  memset(packet, 0, SHUTTLE_VFD_PACKET_SIZE);
  packet[0] = (0xD << 4) + 7;
  packet[1] = DEC_AS_HEX(now->tm_sec);      // sec
  packet[2] = DEC_AS_HEX(now->tm_min);      // min
  packet[3] = DEC_AS_HEX(now->tm_hour);     // hours
  packet[4] = now->tm_wday;                 // day-of-week (1=monday)
  packet[5] = DEC_AS_HEX(now->tm_mday);     // day
  packet[6] = DEC_AS_HEX(now->tm_mon+1);    // month
  packet[7] = DEC_AS_HEX(now->tm_year-100); // year
  vfd_send_packet(packet);

  memset(packet, 0, SHUTTLE_VFD_PACKET_SIZE);
  packet[0] = (3 << 4) + 1;
  packet[1] = 3;
  return vfd_send_packet(packet);
}


/* Simple text display (full screen write, no cursor management) */
int vfd_display_text(const char *text, unsigned int len, const useconds_t delai)
{
  unsigned char packet[SHUTTLE_VFD_PACKET_SIZE];
  int i;
  char *p = (char *)text;

  if (len > SHUTTLE_VFD_WIDTH) {
    len = SHUTTLE_VFD_WIDTH;
  }

  for (i = 0; i < (len/SHUTTLE_VFD_DATA_SIZE); i++) {
    packet[0] = (9 << 4) + SHUTTLE_VFD_DATA_SIZE;
    memcpy(packet + 1, p, SHUTTLE_VFD_DATA_SIZE);
    p += SHUTTLE_VFD_DATA_SIZE;
    vfd_send_packet(packet);
  }

  len = len % SHUTTLE_VFD_DATA_SIZE;
  if (len != 0) {
    memset(packet, 0, SHUTTLE_VFD_PACKET_SIZE);
    packet[0] = (9 << 4) + len;
    memcpy(packet + 1, p, len);
    vfd_send_packet(packet);
  }

  if (delai)
    usleep(delai);

  return 0;
}


int vfd_display_icons(unsigned long value)
{
  unsigned char packet[SHUTTLE_VFD_PACKET_SIZE];
  memset(packet, 0, SHUTTLE_VFD_PACKET_SIZE);
  packet[0] = (7 << 4) + 4;
  packet[1] = (value >> 15) & 0x1F;
  packet[2] = (value >> 10) & 0x1F;
  packet[3] = (value >>  5) & 0x1F;
  packet[4] = value & 0x1F; // each data byte is stored on 5 bits
  return vfd_send_packet(packet);
}


int vfd_parse_icons(const char *name, unsigned long *val)
{
  struct vfd_icons {
    char *name, *altname;
    unsigned long value;
  };

  int i;

  static struct vfd_icons icons[] = {
    { "clk", "clock",   SHUTTLE_VFD_ICON_CLOCK},
    { "rad", "radio",   SHUTTLE_VFD_ICON_RADIO},
    { "mus", "music",   SHUTTLE_VFD_ICON_MUSIC},
    { "cd",  "dvd",     SHUTTLE_VFD_ICON_CD_DVD},
    { "tv",  "tele",    SHUTTLE_VFD_ICON_TELEVISION},
    { "cam", "camera",  SHUTTLE_VFD_ICON_CAMERA},
    { "rew", "rewind",  SHUTTLE_VFD_ICON_REWIND},
    { "rec", "record",  SHUTTLE_VFD_ICON_RECORD},
    { "pl",  "play",    SHUTTLE_VFD_ICON_PLAY},
    { "pa",  "pause",   SHUTTLE_VFD_ICON_PAUSE},
    { "st",  "stop",    SHUTTLE_VFD_ICON_STOP},
    { "ff",  NULL,      SHUTTLE_VFD_ICON_FASTFORWARD},
    { "rev", "reverse", SHUTTLE_VFD_ICON_REVERSE},
    { "rep", "repeat",  SHUTTLE_VFD_ICON_REPEAT},
    { "mute", "vol0",   SHUTTLE_VFD_ICON_MUTE},
    { "all", "world",   SHUTTLE_VFD_ALL_ICONS}
  };

  *val = 0;

  for (i = 0; i < sizeof(icons)/sizeof(struct vfd_icons); i++)
  {
    if (strcmp(name, icons[i].name) == 0) {
      *val = icons[i].value;
    } else if ((icons[i].altname != NULL) && (strcmp(name, icons[i].altname) == 0)) {
      *val = icons[i].value;
    } else if ((strncmp(name, "vol", 3) == 0) && (strlen(name) == 4) &&
        (name[3] > 0x30 && name[3] <= 0x39)) {
      *val = (name[3] - 0x30) << 15;
    } else if ((strncmp(name, "vol1", 4) == 0) && (strlen(name) == 5) &&
        (name[4] >= 0x30 && name[4] <= 0x32)) {
      *val = (name[4] - 0x30 + 10) << 15;
    } else {
      continue;
    }
    break;
  }

  return ((*val == 0) ? -1 : 0);
}
