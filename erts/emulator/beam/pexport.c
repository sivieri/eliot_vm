/*
 * %CopyrightBegin%
 *
 * Copyright Ericsson AB 1996-2010. All Rights Reserved.
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
 * Manage registered processes.
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#include "hash.h"
#include "atom.h"
#include "pexport.h"
#include "register.h"

static Hash process_exp;

#define PEXP_HASH_SIZE 10

#define EXP_HASH(term) ((HashValue) atom_val(term))

static erts_smp_rwmtx_t exptab_rwmtx;

#define exp_try_read_lock()		erts_smp_rwmtx_tryrlock(&exptab_rwmtx)
#define exp_try_write_lock()		erts_smp_rwmtx_tryrwlock(&exptab_rwmtx)
#define exp_read_lock()			erts_smp_rwmtx_rlock(&exptab_rwmtx)
#define exp_write_lock()		erts_smp_rwmtx_rwlock(&exptab_rwmtx)
#define exp_read_unlock()		erts_smp_rwmtx_runlock(&exptab_rwmtx)
#define exp_write_unlock()		erts_smp_rwmtx_rwunlock(&exptab_rwmtx)

#ifdef ERTS_SMP
static ERTS_INLINE void
exp_safe_read_lock(Process *c_p, ErtsProcLocks *c_p_locks)
{
    if (*c_p_locks) {
	ASSERT(c_p);
	ASSERT(c_p_locks);
	ASSERT(*c_p_locks);

	if (exp_try_read_lock() != EBUSY) {
#ifdef ERTS_ENABLE_LOCK_CHECK
	    erts_proc_lc_might_unlock(c_p, *c_p_locks);
#endif
	    return;
	}

	/* Release process locks in order to avoid deadlock */
	erts_smp_proc_unlock(c_p, *c_p_locks);
	*c_p_locks = 0;
    }

    exp_read_lock();
}

static ERTS_INLINE void
exp_safe_write_lock(Process *c_p, ErtsProcLocks *c_p_locks)
{
    if (*c_p_locks) {
	ASSERT(c_p);
	ASSERT(c_p_locks);
	ASSERT(*c_p_locks);

	if (exp_try_write_lock() != EBUSY) {
#ifdef ERTS_ENABLE_LOCK_CHECK
	    erts_proc_lc_might_unlock(c_p, *c_p_locks);
#endif
	    return;
	}

	/* Release process locks in order to avoid deadlock */
	erts_smp_proc_unlock(c_p, *c_p_locks);
	*c_p_locks = 0;
    }

    exp_write_lock();
}

static ERTS_INLINE int
is_proc_alive(Process *p)
{
    int res;
    erts_pix_lock_t *pixlck = ERTS_PID2PIXLOCK(p->id);
    erts_pix_lock(pixlck);
    res = !p->is_exiting;
    erts_pix_unlock(pixlck);
    return res;
}

#endif

static HashValue exp_hash(ExpProc* obj)
{
    if (obj->name == am_undefined) return EXP_HASH(obj->pid);
    else return EXP_HASH(obj->name);
}

static int exp_cmp(ExpProc *tmpl, ExpProc *obj) {
    return tmpl->name != obj->name && tmpl->pid != obj->pid;
}

static ExpProc* exp_alloc(ExpProc *tmpl)
{
    ExpProc* obj = (ExpProc*) erts_alloc(ERTS_ALC_T_REG_PROC, sizeof(ExpProc));
    if (!obj) {
	erl_exit(1, "Can't allocate %d bytes of memory\n", sizeof(ExpProc));
    }
    obj->name = tmpl->name;
    obj->pid = tmpl->pid;
    return obj;
}

static void exp_free(ExpProc *obj)
{
    erts_free(ERTS_ALC_T_REG_PROC, (void*) obj);
}

void init_pexport_table(void)
{
    HashFunctions f;
    erts_smp_rwmtx_opt_t rwmtx_opt = ERTS_SMP_RWMTX_OPT_DEFAULT_INITER;
    rwmtx_opt.type = ERTS_SMP_RWMTX_TYPE_FREQUENT_READ;
    rwmtx_opt.lived = ERTS_SMP_RWMTX_LONG_LIVED;

    erts_smp_rwmtx_init_opt(&exptab_rwmtx, &rwmtx_opt, "exp_tab");

    f.hash = (H_FUN) exp_hash;
    f.cmp  = (HCMP_FUN) exp_cmp;
    f.alloc = (HALLOC_FUN) exp_alloc;
    f.free = (HFREE_FUN) exp_free;

    hash_init(ERTS_ALC_T_REG_TABLE, &process_exp, "process_exp",
	      PEXP_HASH_SIZE, f);
}

int erts_export_process_by_name(Process *c_p, Eterm name)
{
    ExpProc r;
    ERTS_SMP_CHK_HAVE_ONLY_MAIN_PROC_LOCK(c_p);

    if ((is_not_atom(name) || name == am_undefined)) {
        return 0;
    }
    
    r.name = name;
    r.pid = am_undefined;
    
    exp_write_lock();
    hash_put(&process_exp, (void*) &r);
    exp_write_unlock();
    
    return 1;
}

int erts_export_process_by_pid(Process *c_p, Eterm id)
{
    ExpProc r;
    ERTS_SMP_CHK_HAVE_ONLY_MAIN_PROC_LOCK(c_p);

    if (id == am_undefined) {
        return 0;
    }
    
    r.pid = id;
    r.name = am_undefined;

    exp_write_lock();
    hash_put(&process_exp, (void*) &r);
    exp_write_unlock();
    
    return 1;
}

int erts_unexport_process_by_pid(Process *c_p,
			 ErtsProcLocks c_p_locks,
			 Port *c_prt,
			 Eterm id)
{
    int res = 0;
    ExpProc r;

    r.pid = id;
    r.name = am_undefined;

    exp_write_lock();
    if (hash_get(&process_exp, (void*) &r) != NULL) {
        hash_erase(&process_exp, (void*) &r);
        res = 1;
    }
    else {
        res = 0;
    }
    exp_write_unlock();

    return res;
}

int erts_unexport_process_by_name(Process *c_p,
             ErtsProcLocks c_p_locks,
             Port *c_prt,
             Eterm name)
{
    int res = 0;
    ExpProc r;

    r.pid = am_undefined;
    r.name = name;

    exp_write_lock();
    if (hash_get(&process_exp, (void*) &r) != NULL) {
        hash_erase(&process_exp, (void*) &r);
        res = 1;
    }
    else {
        res = 0;
    }
    exp_write_unlock();

    return res;
}

int process_exp_size(void)
{
    int size;
    int lock = !ERTS_IS_CRASH_DUMPING;
    if (lock)
	exp_read_lock();
    size = process_exp.size;
    if (lock)
	exp_read_unlock();
    return size;
}

int erts_is_exported_by_name(Process *c_p, Eterm name)
{
    int res = 0;
    ExpProc r;

    r.pid = am_undefined;
    r.name = name;

    exp_read_lock();
    if (hash_get(&process_exp, (void*) &r) != NULL) {
        res = 1;
    }
    else {
        res = 0;
    }
    exp_read_unlock();

    return res;
}

int erts_is_exported_by_pid(Process *c_p, Eterm id)
{
    int res = 0;
    ExpProc r;

    r.pid = id;
    r.name = am_undefined;

    exp_read_lock();
    if (hash_get(&process_exp, (void*) &r) != NULL) {
        res = 1;
    }
    else {
        res = 0;
    }
    exp_read_unlock();

    return res;
}

Eterm exported_noproc(Eterm* buf) {
    int i;
    Eterm res;
    HashBucket **bucket;

    bucket = process_exp.bucket;

     /* scan through again and make the list */ 
    res = NIL;

    for (i = 0; i < process_exp.size; i++) {
    HashBucket *b = bucket[i];
    while (b != NULL) {
        ExpProc *exp = (ExpProc *) b;
        if (exp->name == am_undefined)
            res = CONS(buf, exp->pid, res);
        else
            res = CONS(buf, exp->name, res);
        buf += 2;
        b = b->next;
    }
    }

    exp_read_unlock();

    return res;
}

/**********************************************************************/

#include "bif.h"

BIF_RETTYPE exported_0(BIF_ALIST_0)
{
    int i;
    Eterm res;
    Uint need;
    Eterm* hp;
    HashBucket **bucket;
#ifdef ERTS_SMP
    ErtsProcLocks proc_locks = ERTS_PROC_LOCK_MAIN;

    ERTS_SMP_CHK_HAVE_ONLY_MAIN_PROC_LOCK(BIF_P);
    exp_safe_read_lock(BIF_P, &proc_locks);
    if (!proc_locks)
	erts_smp_proc_lock(BIF_P, ERTS_PROC_LOCK_MAIN);
#endif

    bucket = process_exp.bucket;

    /* work out how much heap we need & maybe garb, by scanning through
       the registered process table */
    need = 0;
    for (i = 0; i < process_exp.size; i++) {
	HashBucket *b = bucket[i];
	while (b != NULL) {
	    need += 2;
	    b = b->next;
	}
    }

    if (need == 0) {
	exp_read_unlock();
	BIF_RET(NIL);
    }

    hp = HAlloc(BIF_P, need);
     
     /* scan through again and make the list */ 
    res = NIL;

    for (i = 0; i < process_exp.size; i++) {
	HashBucket *b = bucket[i];
	while (b != NULL) {
	    ExpProc *exp = (ExpProc *) b;
	    if (exp->name == am_undefined)
	        res = CONS(hp, exp->pid, res);
	    else
	        res = CONS(hp, exp->name, res);
	    hp += 2;
	    b = b->next;
	}
    }

    exp_read_unlock();

    BIF_RET(res);
}
