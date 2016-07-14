/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2016, Eduardo Silva P. <eduardo@monkey.io>.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef DUDA_H
#define DUDA_H

#define _GNU_SOURCE

#include <monkey/mk_core.h>
#include <monkey/mk_lib.h>

#include "duda_info.h"
#include "duda_version.h"
#include "duda_lib.h"
#include "duda.h"
#include "duda_api.h"
#include "duda_objects.h"
#include "duda_service.h"
#include "duda_request.h"

/* Special headers for web services only */
#ifndef DUDA_LIB_CORE
#include "duda_bootstrap.h"
#include "duda/duda_service_internal.h"
#endif /* !DUDA_LIB_CORE */

#endif
