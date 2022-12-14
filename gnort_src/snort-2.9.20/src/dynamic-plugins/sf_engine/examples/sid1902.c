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

#include "pcre.h"
#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


/* declare detection functions */
int rule1902eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_server; */
static FlowFlags rule1902flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule1902option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &rule1902flow0 }
};
// content:"LSUB", nocase;
static ContentInfo rule1902content1 =
{
    (u_int8_t *)"LSUB", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption rule1902option1 =
{
    OPTION_TYPE_CONTENT,
    { &rule1902content1 }
};
// pcre:"\sLSUB\s[^\n]*?\s\{", dotall, multiline, nocase;
static PCREInfo rule1902pcre2 =
{
    "\\sLSUB\\s[^\\n]*?\\s\\{", /* pattern */
    NULL,                               /* holder for compiled pattern */
    NULL,                               /* holder for compiled pattern flags */
    PCRE_CASELESS|PCRE_DOTALL|PCRE_MULTILINE,     /* compile flags */
    CONTENT_BUF_NORMALIZED,     /* content flags */
    0 /* offset */
};

static RuleOption rule1902option2 =
{
    OPTION_TYPE_PCRE,
    { &rule1902pcre2 }
};
/* byte_test:size 5, value 256, operator >, relative, representation dec; */
static ByteData rule1902byte_test3 =
{
    5, /* size */
    CHECK_GT, /* operator */
    256, /* value */
    0, /* offset */
    0, /*multiplier */
    BYTE_BIG_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_DEC|EXTRACT_AS_STRING, /* flags */
    0, /* offset */
    NULL, // offset_location
    NULL, // value_location
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule1902option3 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule1902byte_test3 }
};

/* references for sid 1902 */
/* reference: bugtraq "1110"; */
static RuleReference rule1902ref1 =
{
    "bugtraq", /* type */
    "1110" /* value */
};

/* reference: cve "2000-0284"; */
static RuleReference rule1902ref2 =
{
    "cve", /* type */
    "2000-0284" /* value */
};

/* reference: nessus "10374"; */
static RuleReference rule1902ref3 =
{
    "nessus", /* type */
    "10374" /* value */
};

static RuleReference *rule1902refs[] =
{
    &rule1902ref1,
    &rule1902ref2,
    &rule1902ref3,
    NULL
};
RuleOption *rule1902options[] =
{
    &rule1902option0,
    &rule1902option1,
    &rule1902option2,
    &rule1902option3,
    NULL
};

Rule rule1902 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "143", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       1902, /* sigid */
       9, /* revision */

       "misc-attack", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "IMAP lsub literal overflow attempt",     /* message */
       rule1902refs, /* ptr to references */
       NULL /* Meta data */
   },
   rule1902options, /* ptr to rule options */
   NULL, // &rule1902eval, /* use the built in detection function */
   0, /* am I initialized yet? */
   0,                                  /* Rule option count, used internally */
   0,                                  /* Flag with no alert, used internally */
   NULL /* ptr to internal data... setup during rule registration */
};


/* detection functions */
int rule1902eval(void *p) {
    //const u_int8_t *cursor_uri = 0;
    //const u_int8_t *cursor_raw = 0;
    const u_int8_t *cursor_normal = 0;


    // flow:established, to_server;
    if (checkFlow(p, rule1902options[0]->option_u.flowFlags) > 0 ) {
        // content:"LSUB", nocase;
        if (contentMatch(p, rule1902options[1]->option_u.content, &cursor_normal) > 0) {
            // pcre:"\sLSUB\s[^\n]*?\s\{", dotall, multiline, nocase;
            if (pcreMatch(p, rule1902options[2]->option_u.pcre, &cursor_normal)) {
                // byte_test:size 5, value 256, operator >, relative, representation dec;
                if (byteTest(p, rule1902options[3]->option_u.byte, cursor_normal) > 0) {
                    return RULE_MATCH;
                }
            }
        }
    }
    return RULE_NOMATCH;
}
