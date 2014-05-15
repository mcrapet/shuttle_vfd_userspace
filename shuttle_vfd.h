/*
 * shuttle_vfd.h - Low-level communication layer.
 * Copyright (C) 2008 Matthieu Crapet <mcrapet@gmail.com>
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

#ifndef SHUTTLE_VFD_H
#define SHUTTLE_VFD_H

#include <time.h>

// VFD USB properties
#define SHUTTLE_VFD_VENDOR_ID  0x051C
#define SHUTTLE_VFD_PRODUCT_ID 0x0005 // IR-receiver included
#define SHUTTLE_VFD_INTERFACE_NUM   1

// VFD physical dimensions
#define SHUTTLE_VFD_WIDTH          20
#define SHUTTLE_VFD_HEIGHT          1  // not used

// VFD USB control message
#define SHUTTLE_VFD_PACKET_SIZE         8
#define SHUTTLE_VFD_DATA_SIZE           (SHUTTLE_VFD_PACKET_SIZE-1)
#define SHUTTLE_VFD_WRITE_ATTEMPTS      2
#define SHUTTLE_VFD_SUCCESS_SLEEP_USEC  25600
#define SHUTTLE_VFD_RETRY_SLEEP_USEC    25600

// VFD Icons
#define SHUTTLE_VFD_ICON_CLOCK          (1 << 4)
#define SHUTTLE_VFD_ICON_RADIO          (1 << 3)
#define SHUTTLE_VFD_ICON_MUSIC          (1 << 2)
#define SHUTTLE_VFD_ICON_CD_DVD         (1 << 1)
#define SHUTTLE_VFD_ICON_TELEVISION     (1 << 0)
#define SHUTTLE_VFD_ICON_CAMERA         (1 << 9)
#define SHUTTLE_VFD_ICON_REWIND         (1 << 8)
#define SHUTTLE_VFD_ICON_RECORD         (1 << 7)
#define SHUTTLE_VFD_ICON_PLAY           (1 << 6)
#define SHUTTLE_VFD_ICON_PAUSE          (1 << 5)
#define SHUTTLE_VFD_ICON_STOP           (1 << 14)
#define SHUTTLE_VFD_ICON_FASTFORWARD    (1 << 13)
#define SHUTTLE_VFD_ICON_REVERSE        (1 << 12)
#define SHUTTLE_VFD_ICON_REPEAT         (1 << 11)
#define SHUTTLE_VFD_ICON_MUTE           (1 << 10)
#define SHUTTLE_VFD_ICON_VOL_01         (1 << 15)
#define SHUTTLE_VFD_ICON_VOL_02         (2 << 15)
#define SHUTTLE_VFD_ICON_VOL_03         (3 << 15)
#define SHUTTLE_VFD_ICON_VOL_04         (4 << 15)
#define SHUTTLE_VFD_ICON_VOL_05         (5 << 15)
#define SHUTTLE_VFD_ICON_VOL_06         (6 << 15)
#define SHUTTLE_VFD_ICON_VOL_07         (7 << 15)
#define SHUTTLE_VFD_ICON_VOL_08         (8 << 15)
#define SHUTTLE_VFD_ICON_VOL_09         (9 << 15)
#define SHUTTLE_VFD_ICON_VOL_10         (10 << 15)
#define SHUTTLE_VFD_ICON_VOL_11         (11 << 15)
#define SHUTTLE_VFD_ICON_VOL_12         (12 << 15)

#define SHUTTLE_VFD_ALL_ICONS           (0x7FFF|SHUTTLE_VFD_ICON_VOL_12)

/* Prototypes */

int vfd_init(int, int, int);
int vfd_close(int);
int vfd_send_packet(unsigned char packet[SHUTTLE_VFD_PACKET_SIZE]);
int vfd_clear(int);
int vfd_display_clock(void);
int vfd_display_text(const char *, unsigned int, const useconds_t);
int vfd_display_icons(unsigned long);
int vfd_parse_icons(const char *, unsigned long *);

#endif /* SHUTTLE_VFD_H */
