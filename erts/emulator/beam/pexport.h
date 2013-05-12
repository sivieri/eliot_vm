/*
 * %CopyrightBegin%
 * 
 * Copyright Ericsson AB 1996-2009. All Rights Reserved.
 * 
 * The contents of this file are subject to the Erlang Public License,
 * Version 1.1, (the "License"); you may not use this file except in
 * compliance with the License. You should have received a copy of the
 * Erlang Public License along with this software. If not, it can be
 * retrieved online at http://www.erlang.org/.
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 * 
 * %CopyrightEnd%
 */

/*
** Exported processes
*/

#ifndef __PEXPORT_H__
#define __PEXPORT_H__

#ifndef __SYS_H__
#include "sys.h"
#endif

#ifndef __HASH_H__
#include "hash.h"
#endif

#ifndef __PROCESS_H__
#include "erl_process.h"
#endif

typedef struct exp_proc
{
    HashBucket bucket;  /* MUST BE LOCATED AT TOP OF STRUCT!!! */
    Eterm pid;
    Eterm name;
} ExpProc;

int process_exp_size(void);
void init_pexport_table(void);
int erts_export_process_by_name(Process *, Eterm);
int erts_export_process_by_pid(Process *, Eterm);
int erts_unexport_process_by_name(Process *, ErtsProcLocks, struct port *, Eterm);
int erts_unexport_process_by_pid(Process *, ErtsProcLocks, struct port *, Eterm);
int erts_is_exported_by_name(Process *, Eterm);
int erts_is_exported_by_pid(Process *, Eterm);
Eterm exported_noproc(Eterm*);

#endif
