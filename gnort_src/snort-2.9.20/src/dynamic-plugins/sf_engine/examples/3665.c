/*
 * VRT RULES
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * This file is autogenerated via rules2c, by Brian Caswell <bmc@sourcefire.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "detection_lib_meta.h"

/* declare detection functions */
int rule3665eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow for sid 3665 */
/* flow:established, from_server; */
static FlowFlags rule3665flow1 =
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule3665option1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &rule3665flow1 }
};

/* content for sid 3665 */
// content:"|00|", offset 3, depth 1;
static ContentInfo rule3665content2 =
{
    (u_int8_t *)("|00|"), /* pattern (now in snort content format) */
    1, /* depth */
    3, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length*/
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption rule3665option2 =
{
    OPTION_TYPE_CONTENT,
    { &rule3665content2 }
};

/* flowbits for sid 3665 */
/* flowbits:set "mysql.server_greeting"; */
static FlowBitsInfo rule3665flowbits3 =
{
    "mysql.server_greeting",
    FLOWBIT_SET,
    0,
    0, /* flags */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption rule3665option3 =
{
    OPTION_TYPE_FLOWBIT,
    { &rule3665flowbits3 }
};

/* references for sid 3665 */
static RuleReference *rule3665refs[] =
{
    NULL
};

RuleOption *rule3665options[] =
{
    &rule3665option1,
    &rule3665option2,
    &rule3665option3,
    NULL
};

Rule rule3665 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$SQL_SERVERS", /* SRCIP     */
       "3306", /* SRCPORT   */
       0, /* DIRECTION */
       "$EXTERNAL_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       3665, /* sigid */
       1, /* revision */

       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "MYSQL server greeting",     /* message */
       rule3665refs, /* ptr to references */
       NULL /* Meta data */
   },
   rule3665options, /* ptr to rule options */
   NULL,                               /* Use internal eval func */
   0,                                  /* Not initialized */
   0,                                  /* Rule option count, used internally */
   0,                                  /* Flag with no alert, used internally */
   NULL /* ptr to internal data... setup during rule registration */
};




/* detection functions */

int rule3665eval(void *p) {
    /* cursors, formally known as doe_ptr */
    const u_int8_t *cursor_normal = 0;

    // flow:established, from_server;
    if (checkFlow(p, rule3665options[0]->option_u.flowFlags) > 0 ) {
        // content:"|00|", offset 3, depth 1;
        if (contentMatch(p, rule3665options[1]->option_u.content, &cursor_normal) > 0) {
            // flowbits:set "mysql.server_greeting";
            if (processFlowbits(p, rule3665options[2]->option_u.flowBit) > 0) {
                printf("not alerting, but thats ok!\n");
                    // flowbits:noalert;
                /* NOTE: Not including any further functions for testing! */
                return RULE_NOMATCH;
            }
        }
    }
    return RULE_NOMATCH;
}

