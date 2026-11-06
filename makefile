# SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
# Copyright (c) 2009-2026 John R. Smythe Jr.
#
include		../rules.mak
CHDRS =	$(HDRS:%.h=$(HDR)/%.h)
LIBRARIES =
include		../targets.mak
include	.depend
