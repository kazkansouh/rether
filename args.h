/*
 * Copyright (c) 2019 Karim Kanso. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined(_ARGS_H)

#include <stdbool.h>
#include <stdint.h>
#include <net/if.h>

extern const uint16_t gui_ethertype;
extern const uint8_t gui_destmac[6];
extern const char gpch_source_interface[IFNAMSIZ];
extern const uint8_t * const gui_data;
extern const size_t gz_data;
extern const bool gb_receive;

bool args_parse(int argc, const char** argv);

#endif // _ARGS_H
