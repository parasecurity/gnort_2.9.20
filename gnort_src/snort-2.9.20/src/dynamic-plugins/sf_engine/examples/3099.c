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


/* declare detection functions */
int rule3099eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_server; */
static FlowFlags rule3099flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule3099option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &rule3099flow0 }
};
// content:"|00|", depth 1;
static ContentInfo rule3099content1 =
{
    (u_int8_t *)("|00|"), /* pattern (now in snort content format) */
    1, /* depth */
    0, /* offset */
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

static RuleOption rule3099option1 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content1 }
};
// content:"|FF|SMB%", offset 3, depth 5, relative;
static ContentInfo rule3099content2 =
{
    (u_int8_t *)("|FF|SMB%"), /* pattern (now in snort content format) */
    5, /* depth */
    3, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3099option2 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content2 }
};
/* byte_test:size 1, value 128, operator &, offset 6, relative; */
static ByteData rule3099byte_test3 =
{
    1, /* size */
    CHECK_AND, /* operator */
    128, /* value */
    6, /* offset */
    0, /*multiplier */
    NOT_FLAG|BYTE_BIG_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
    0, /* post offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule3099option3 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule3099byte_test3 }
};
// pcre:"^.{27}", relative;
static PCREInfo rule3099pcre4 =
{
    "^.{27}", /* pattern */
    NULL,                               /* holder for compiled pattern */
    NULL,                               /* holder for compiled pattern flags */
    0,     /* compile flags */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED,     /* content flags */
    0 /* offset */
};

static RuleOption rule3099option4 =
{
    OPTION_TYPE_PCRE,
    { &rule3099pcre4 }
};
// content:"&|00|", offset 29, depth 2, relative;
static ContentInfo rule3099content5 =
{
    (u_int8_t *)("&|00|"), /* pattern (now in snort content format) */
    2, /* depth */
    29, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3099option5 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content5 }
};
// content:"|5C|PIPE|5C 00|", offset 4, nocase, relative;
static ContentInfo rule3099content6 =
{
    (u_int8_t *)("|5C|PIPE|5C 00|"), /* pattern (now in snort content format) */
    0, /* depth */
    4, /* offset */
    CONTENT_NOCASE|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3099option6 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content6 }
};
/* byte_jump:size 2, offset -17, relative, endian little; */
static ByteData rule3099byte_jump7 =
{
    2, /* size */
    0, /* operator, byte_jump doesn't use operator! */
    0, /* value, byte_jump doesn't use value! */
    -17, /* offset */
    0, /* multiplier */
    BYTE_LITTLE_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE|JUMP_FROM_BEGINNING, /* flags */
    0, /* post offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule3099option7 =
{
    OPTION_TYPE_BYTE_JUMP,
    { &rule3099byte_jump7 }
};
// pcre:"^.{4}", relative;
static PCREInfo rule3099pcre8 =
{
    "^.{4}", /* pattern */
    NULL,                               /* holder for compiled pattern */
    NULL,                               /* holder for compiled pattern flags */
    0,     /* compile flags */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED,     /* content flags */
    0 /* offset */
};

static RuleOption rule3099option8 =
{
    OPTION_TYPE_PCRE,
    { &rule3099pcre8 }
};
// content:"|05|", depth 1, relative;
static ContentInfo rule3099content9 =
{
    (u_int8_t *)("|05|"), /* pattern (now in snort content format) */
    1, /* depth */
    0, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3099option9 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content9 }
};
/* byte_test:size 1, value 16, operator &, offset 3, relative; */
static ByteData rule3099byte_test10 =
{
    1, /* size */
    CHECK_AND, /* operator */
    16, /* value */
    3, /* offset */
    0, /*multiplier */
    BYTE_BIG_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
    0, /* post offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule3099option10 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule3099byte_test10 }
};
// content:"|0B|", offset 1, depth 1, relative;
static ContentInfo rule3099content11 =
{
    (u_int8_t *)("|0B|"), /* pattern (now in snort content format) */
    1, /* depth */
    1, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3099option11 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content11 }
};
// content:"@|FD|,4l<|CE 11 A8 93 08 00|+.|9C|m", offset 29, depth 16, relative;
static ContentInfo rule3099content12 =
{
    (u_int8_t *)("@|FD|,4l<|CE 11 A8 93 08 00|+.|9C|m"), /* pattern (now in snort content format) */
    16, /* depth */
    29, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|CONTENT_FAST_PATTERN, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3099option12 =
{
    OPTION_TYPE_CONTENT,
    { &rule3099content12 }
};
/* flowbits:set "smb.tree.bind.llsrpc"; */
static FlowBitsInfo rule3099flowbits13 =
{
    "smb.tree.bind.llsrpc",
    FLOWBIT_SET,
    0,
    0, /* flags */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption rule3099option13 =
{
    OPTION_TYPE_FLOWBIT,
    { &rule3099flowbits13 }
};

/* references for sid 3099 */
static RuleReference *rule3099refs[] =
{
    NULL
};
RuleOption *rule3099options[] =
{
    &rule3099option0,
    &rule3099option1,
    &rule3099option2,
    &rule3099option3,
    &rule3099option4,
    &rule3099option5,
    &rule3099option6,
    &rule3099option7,
    &rule3099option8,
    &rule3099option9,
    &rule3099option10,
    &rule3099option11,
    &rule3099option12,
    &rule3099option13,
    NULL
};

Rule rule3099 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       1, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "139", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       3099, /* sigid */
       2, /* revision */

       "protocol-command-decode", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "NETBIOS SMB llsrpc little endian bind attempt",     /* message */
       rule3099refs, /* ptr to references */
       NULL /* Meta data */
   },
   rule3099options, /* ptr to rule options */
   NULL, // &rule3099eval, /* use the built in detection function */
   0, /* am I initialized yet? */
   0,                                  /* Rule option count, used internally */
   0,                                  /* Flag with no alert, used internally */
   NULL /* ptr to internal data... setup during rule registration */
};


/* detection functions */
int rule3099eval(void *p) {
    //const u_int8_t *cursor_uri = 0;
    //const u_int8_t *cursor_raw = 0;
    const u_int8_t *cursor_normal = 0;


    // flow:established, to_server;
    if (checkFlow(p, rule3099options[0]->option_u.flowFlags) > 0 ) {
        // content:"|00|", depth 1;
        if (contentMatch(p, rule3099options[1]->option_u.content, &cursor_normal) > 0) {
            // content:"|FF|SMB%", offset 3, depth 5, relative;
            if (contentMatch(p, rule3099options[2]->option_u.content, &cursor_normal) > 0) {
                // byte_test:size 1, value 128, operator &, offset 6, relative;
                if (!(byteTest(p, rule3099options[3]->option_u.byte, cursor_normal) > 0)) {
                    // pcre:"^.{27}", relative;
                    if (pcreMatch(p, rule3099options[4]->option_u.pcre, &cursor_normal)) {
                        // content:"&|00|", offset 29, depth 2, relative;
                        if (contentMatch(p, rule3099options[5]->option_u.content, &cursor_normal) > 0) {
                            // content:"|5C|PIPE|5C 00|", offset 4, nocase, relative;
                            if (contentMatch(p, rule3099options[6]->option_u.content, &cursor_normal) > 0) {
                                // byte_jump:size 2, offset -17, relative, endian little;
                                if (byteJump(p, rule3099options[7]->option_u.byte, &cursor_normal) > 0) {
                                    // pcre:"^.{4}", relative;
                                    if (pcreMatch(p, rule3099options[8]->option_u.pcre, &cursor_normal)) {
                                        // content:"|05|", depth 1, relative;
                                        if (contentMatch(p, rule3099options[9]->option_u.content, &cursor_normal) > 0) {
                                            // byte_test:size 1, value 16, operator &, offset 3, relative;
                                            if (byteTest(p, rule3099options[10]->option_u.byte, cursor_normal) > 0) {
                                                // content:"|0B|", offset 1, depth 1, relative;
                                                if (contentMatch(p, rule3099options[11]->option_u.content, &cursor_normal) > 0) {
                                                    // content:"@|FD|,4l<|CE 11 A8 93 08 00|+.|9C|m", offset 29, depth 16, relative;
                                                    if (contentMatch(p, rule3099options[12]->option_u.content, &cursor_normal) > 0) {
                                                        // flowbits:set "smb.tree.bind.llsrpc";
                                                        if (processFlowbits(p, rule3099options[13]->option_u.flowBit) > 0) {
                                                                return RULE_MATCH;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return RULE_NOMATCH;
}

