//
// Created by 白杰 on 2025/8/8.
//

#ifndef PG_SNPDUMP_PG_CLASS_H
#define PG_SNPDUMP_PG_CLASS_H

#include "pg_common.h"
#include <iostream>
#include <cstdio>

using namespace std;


int resolvePgClassHeapData(char*, const char*, unsigned int*);

// pg_class
typedef struct {
    /* oid */
    Oid 			oid;
    /* class name */
    NameData 	relname;
    /* OID of namespace containing this class */
    Oid			relnamespace ;
    /* OID of entry in pg_type for relation's implicit row type, if any */
    Oid			reltype ;
    /* OID of entry in pg_type for underlying composite type, if any */
    Oid			reloftype ;
    /* class owner */
    Oid			relowner ;
    /* access method; 0 if not a table / index */
    Oid			relam ;
    /* identifier of physical storage file */
    /* relfilenode == 0 means it is a "mapped" relation, see relmapper.c */
    Oid			relfilenode;
    /* identifier of table space for relation (0 means default for database) */
    Oid			reltablespace ;
    /* # of blocks (not always up-to-date) */
    uint32_t 		relpages ;
    /* # of tuples (not always up-to-date; -1 means "unknown") */
    float 		reltuples ;
    /* # of all-visible blocks (not always up-to-date) */
    uint32_t		relallvisible ;
    /* OID of toast table; 0 if none */
    Oid			reltoastrelid ;
    /* T if has (or has had) any indexes */
    bool		relhasindex ;
    /* T if shared across databases */
    bool		relisshared ;
    /* see RELPERSISTENCE_xxx constants below */
    char		relpersistence ;
    /* see RELKIND_xxx constants below */
    char		relkind ;
    /* number of user attributes */
    uint16_t		relnatts ;	/* genbki.pl will fill this in */
    /*
     * Class pg_attribute must contain exactly "relnatts" user attributes
     * (with attnums ranging from 1 to relnatts) for this class.  It may also
     * contain entries with negative attnums for system attributes.
     */
    /* # of CHECK constraints for class */
    uint16_t		relchecks ;
    /* has (or has had) any rules */
    bool		relhasrules ;
    /* has (or has had) any TRIGGERs */
    bool		relhastriggers ;
    /* has (or has had) child tables or indexes */
    bool		relhassubclass ;
    /* row security is enabled or not */
    bool		relrowsecurity ;
    /* row security forced for owners or not */
    bool		relforcerowsecurity ;
    /* matview currently holds query results */
    bool		relispopulated ;
    /* see REPLICA_IDENTITY_xxx constants */
    char		relreplident ;
    /* is relation a partition? */
    bool		relispartition ;
    /* link to original rel during table rewrite; otherwise 0 */
    Oid			relrewrite ;
    /* all Xids < this are frozen in this rel */
    TransactionId relfrozenxid ;	/* FirstNormalTransactionId */
    /* all multixacts in this rel are >= this; it is really a MultiXactId */
    TransactionId relminmxid ;	/* FirstMultiXactId */
#ifdef CATALOG_VARLEN			/* variable-length fields start here */
    /* NOTE: These fields are not present in a relcache entry's rd_rel field. */
	/* access permissions */
	aclitem		relacl[1] BKI_DEFAULT(_null_);
	/* access-method-specific options */
	text		reloptions[1] BKI_DEFAULT(_null_);
	/* partition bound node tree */
	pg_node_tree relpartbound BKI_DEFAULT(_null_);
#endif
} FormData_pg_class;

typedef FormData_pg_class *Form_pg_class;

#endif //PG_SNPDUMP_PG_CLASS_H
