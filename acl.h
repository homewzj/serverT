#ifndef __ACL_H__
#define __ACL_H__


#include "common.h"
#include "log.h"

enum RULE_TYPE{
    RULE_IPV4 = 0x1,  //IPV4;
    RULE_IPV6  //IPV6;
};


#define IPV4_TYPE_IP 4 
#define IPV4_TYPE_CIDR  5

#define OPT_GET_START_ADDR 1
#define OPT_GET_END_ADDR 2


void displayAclTree(struct access_control_list_s *aclctx);
int checkClientIpIsNotAlloc(struct access_control_list_s *aclctx, char *pClientIp, logContext *pLogCtx);
struct access_control_list_s * aclctxInit(logContext *pLogCtx);
void destroyAclCtx(struct access_control_list_s *aclctx, logContext *pLogCtx);
struct access_control_list_s * parseWhiteListAclRule(webServerContext *pWebContext);

#endif
