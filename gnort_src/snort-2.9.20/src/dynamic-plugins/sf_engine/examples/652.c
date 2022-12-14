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
int rule652eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* content for sid 652 */
// content:"|90 90 90 E8 C0 FF FF FF|/bin/sh";
static ContentInfo rule652content1 =
{
    (u_int8_t *)("|90 90 90 E8 C0 FF FF FF|/bin/sh"), /* pattern (now in snort content format) */
    0, /* depth */
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

static RuleOption rule652option1 =
{
    OPTION_TYPE_CONTENT,
    { &rule652content1 }
};


/* references for sid 652 */
static RuleReference *rule652refs[] =
{
    NULL
};

RuleOption *rule652options[] =
{
    &rule652option1,
    NULL
};

Rule rule652 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_IP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "$SHELLCODE_PORTS", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       652, /* sigid */
       9, /* revision */

       "shellcode-detect", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SHELLCODE Linux shellcode",     /* message */
       rule652refs, /* ptr to references */
       NULL /* Meta data */
   },
   rule652options, /* ptr to rule options */
    NULL,                               /* Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};

/* detection functions */

int rule652eval(void *p) {
    /* cursors, formally known as doe_ptr */
    const u_int8_t *cursor_normal = 0;

    // content:"|90 90 90 E8 C0 FF FF FF|/bin/sh";
     if (contentMatch(p, rule652options[0]->option_u.content, &cursor_normal) > 0) {
        printf("WORKED\n");
        return RULE_MATCH;
    }
    return RULE_NOMATCH;
}

