#include "acl.h"

static int isAvaliableIpv4(int ipv4[], int size);
static struct access_control_elem_s * parseIpv4Rule(char *pRule, logContext *pLogCtx);
static void showIpv6(unsigned char *ipv6);
static bool isIpv6(const char *ipaddr);
static int clientIpIsIpv4WhiteList(struct access_control_elem_s *node, long ipv4netAddr);
static int clientIpIsIpv6WhiteList(struct access_control_elem_s *node, struct access_control_elem_s *pCheckNode);
static unsigned char *getPrefixStr(int len);
static void getStartOrEndAddr(unsigned char *pOrigin, unsigned char *prefixAddr, unsigned char *ipv6, int type, int len);
static int forEachWhiteListRuleTree(struct access_control_elem_s *root, struct access_control_elem_s *pCheckNode);
static struct access_control_elem_s * insertAclRuleNode(struct access_control_elem_s * root, struct access_control_elem_s * curNode);
static struct access_control_list_s * parseAclRule(struct access_control_list_s * ctx, char *pRule, logContext *pLogCtx);
static void destroyWhiteListTree(struct access_control_elem_s *ctx);
static struct access_control_elem_s * parseIpv6Rule(char *pIpv6Addr, logContext *pLogCtx);
static unsigned char * getFullIpv6(char *pIpv6Addr);
static void displayAclRuleTree(struct access_control_elem_s *root, int type);
static int getValue(int len);

void displayAclTree(struct access_control_list_s *aclctx) {
    if(aclctx){
        printf("-----------------------------IPV4-------------------------------------------\n");
        displayAclRuleTree(aclctx->ipv4, RULE_IPV4);
        printf("-----------------------------IPV6-------------------------------------------\n");
        displayAclRuleTree(aclctx->ipv6, RULE_IPV6);
    }
}

static int getValue(int len) {
    int sum = 0;
    int i;
    for( i = 0; i < len; i++ ){
        sum += 1 << ( 8  - len );
    }
    return sum;
}

static unsigned char * getFullIpv6(char *pIpv6Addr) {
    char *pAddr;
    int n, i, r, k, j;
    short s;
    unsigned char *ipv6 = (unsigned char *)malloc(16);
    if ( !ipv6 ){
        return NULL;
    }
    memset(ipv6, 0x0, 16);
    k = 0;
    pAddr = pIpv6Addr;
    while(1) {
        if ( *pAddr == ':' )
            k++;
        if ( *pAddr == '\0' )
            break;
        pAddr++;
    }
    if ( pIpv6Addr[0] == ':' ) k--;
    i = 0;
    pAddr = pIpv6Addr;
    while(1) {
        r = sscanf(pAddr, "%hx%n", &s, &n);
        if ( r == 1 ) {
            ipv6[i] = *((unsigned char *)&s + 1);
            ipv6[i+1] = *((unsigned char *)(&s));
            i += 2;
            if ( i >= 16 ) break;
            pAddr += n;
        } else if ( r == 0 ) {
            if ( pAddr[0] == ':' ) {
                if ( pAddr[1] == ':') {
                    for ( j = 0; j < (8 - k)*2; j++ ) ipv6[ i + j ] = 0;
                    i += ( 8 - k )*2;
                    if ( i >= 16 ) break;
                    pAddr += 2;
                }else{
                    pAddr++;
                }
            }else {
                break;
            }
        }else
            break;
    }
    return ipv6;
}

struct access_control_list_s *aclctxInit(logContext *pLogCtx) {
    struct access_control_list_s *ctx = (struct access_control_list_s *)malloc(sizeof(struct access_control_list_s));
    if ( !ctx ){
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "sds", __func__, __LINE__, "call malloc occur error:", strerror(errno));
        return NULL;
    }
    ctx->aclEnable=true;
    ctx->ipv4 = NULL;
    ctx->ipv6 = NULL;
    logRecord(pLogCtx, LOG_LEVEL_INFO, "sds", __func__, __LINE__, "init access control list context successfully!");
    return ctx;
}

static int isAvaliableIpv4(int ipv4[], int size) {
    int index = 0;
    int iRet = RET_ERROR;
    for (;index < size; index++) {
        if (ipv4[index] >=0 && ipv4[index]<255) {
            continue;
        }else{
            return iRet;
        }
    }
    return RET_OK;
}

static void showIpv6(unsigned char *ipv6) {
    int i;
    for ( i = 0; i < 16; i++ ) {
        printf("%02x ", ipv6[i]);
    }
    printf("\n");
}

void displayAclRuleTree(struct access_control_elem_s *root, int type){
    struct access_control_elem_s *node = root;
    if( node ) {
        if(node->left){
            displayAclRuleTree(node->left, type);
        }
        if ( type == RULE_IPV4 ) {
            printf("netid:%ld  netaddr:%ld netmask:%ld netmasklen:%d\n",(node->netaddr &node->netmask), node->netaddr, node->netmask, node->netmasklen);
        }else {
            showIpv6(node->ipv6);
            showIpv6(node->ipv6start);
            showIpv6(node->ipv6end);
        }
        if(node->right){
            displayAclRuleTree(node->right, type);
        }
    }
}

static struct access_control_elem_s * parseIpv4Rule(char *pRule, logContext *pLogCtx){
    assert(pRule != NULL&& pLogCtx != NULL);
    int iRet = RET_ERROR;
    struct access_control_elem_s * node = (struct access_control_elem_s *)malloc(sizeof(struct access_control_elem_s));
    if ( NULL == node ) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "sdss", __func__, __LINE__, "call malloc occur error:", strerror(errno));
        return NULL;
    }
    int netmasklen = 0;
    long netaddr = 0;
    long netmask = 0;
    int count = -1;
    int network[5];
    count = sscanf(pRule, "%d.%d.%d.%d/%d", &network[0], &network[1], &network[2], &network[3], &network[4]);
    if ( count == IPV4_TYPE_IP ) {
        network[4] = netmasklen = 32;
    }else if ( count == IPV4_TYPE_CIDR ) {
        netmasklen = network[4];
    }else{
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "sdsss", __func__, __LINE__, "ipv4 addr:", pRule, "format is error");
        return NULL; 
    }

    iRet = isAvaliableIpv4(network, 4);
    if ( iRet == RET_ERROR ) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "sdsss", __func__, __LINE__, "ipv4 addr:", pRule, "format is error");
        return NULL;
    }
    netaddr = ((long)network[0]<<24) + ((long)network[1]<<16) + ((long)network[2]<<8) + (long)network[3];
    netmask = (1L << 32 ) - 1 - (1L <<( 32 - network[4])) + 1;
    node->netaddr = netaddr;
    node->netmask = netmask;
    node->netmasklen = netmasklen;
    node->type = RULE_IPV4;
    node->left = node->right =  NULL;
    node->ipv6start = node->ipv6end = node->ipv6 = NULL;
    return node;
}

static bool isIpv6(const char *ipaddr) {
    char *pstr = strstr(ipaddr, ":");
    return pstr == NULL ? false : true;
}

static int clientIpIsIpv4WhiteList(struct access_control_elem_s *node, long ipv4netAddr){
    int iRet = RET_ERROR;
    if( node == NULL ){
        return iRet;
    }else {
        if(((node->netaddr)&(node->netmask)) == ( ipv4netAddr  & (node->netmask))){
            return RET_OK;
        }
        if ( (ipv4netAddr & node->netmask) < (node->netaddr & node->netmask)) {
            iRet = clientIpIsIpv4WhiteList(node->left, ipv4netAddr);
        }else{
            iRet = clientIpIsIpv4WhiteList(node->right, ipv4netAddr);
        }
    }
    return iRet;
}

static int clientIpIsIpv6WhiteList(struct access_control_elem_s *node, struct access_control_elem_s *pCheckNode) {
    int iRet = RET_ERROR;
    if ( node == NULL ){
        return iRet;
    }else{
        if((memcmp(pCheckNode->ipv6, node->ipv6start, 16) >= 0) && (memcmp(pCheckNode->ipv6end, node->ipv6end, 16) <= 0))
        {
            return RET_OK;
        }
        if (memcmp(pCheckNode->ipv6,node->ipv6, 16) < 0) {
            iRet = clientIpIsIpv6WhiteList(node->left, pCheckNode);
        }else{
            iRet = clientIpIsIpv6WhiteList(node->right, pCheckNode);
        }
    }
    return iRet;
}

int checkClientIpIsNotAlloc(struct access_control_list_s *aclctx, char *pClientIp, logContext *pLogCtx) {
    assert(aclctx != NULL && pClientIp != NULL && pLogCtx != NULL);
    int iRet = RET_ERROR;
    long ipv4Netaddr = 0;
    int ipv4[4];
    logRecord(pLogCtx, LOG_LEVEL_INFO, "sss", "check Client Ip:", pClientIp, "is not allowed to access.");
    struct access_control_elem_s *pCheckNode = NULL;
    if (aclctx && aclctx->aclEnable == true ) {
        bool isipv6 = isIpv6(pClientIp);
        if (isipv6) {
            pCheckNode = parseIpv6Rule(pClientIp, pLogCtx);
            iRet = clientIpIsIpv6WhiteList(aclctx->ipv6, pCheckNode);
            destroyWhiteListTree(pCheckNode);
        } else {
            sscanf(pClientIp, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]);
            ipv4Netaddr = ((long)ipv4[0]<<24) + ((long)ipv4[1]<<16) + ((long)ipv4[2]<<8) + (long)ipv4[3];
            iRet = clientIpIsIpv4WhiteList(aclctx->ipv4, ipv4Netaddr);
        }  
        return iRet;
    }
    return RET_OK; 
}

static struct access_control_elem_s * parseIpv6Rule(char *pIpv6Addr, logContext *pLogCtx){
    assert(pIpv6Addr != NULL && pLogCtx != NULL);
    unsigned char *ipv6start = NULL;
    unsigned char *ipv6end = NULL;
    unsigned char *pPrefixStr = NULL;
    struct access_control_elem_s * node = (struct access_control_elem_s *)malloc(sizeof(struct access_control_elem_s));
    if(NULL == node) {
        logRecord(pLogCtx, LOG_LEVEL_ERROR, "sdsss", __func__, __LINE__, "ipv6 addr:", pIpv6Addr, "format is error");
        return NULL;
    }
    char *pIsCidr = NULL;
    int  prefixlen = 0;
    pIsCidr = strstr(pIpv6Addr, "/");
    if ( NULL == pIsCidr ) {
        prefixlen = 128;
    } else {
        prefixlen = atoi(pIsCidr + 1);
    }
    unsigned char *ipv6 = getFullIpv6(pIpv6Addr);
    node->netaddr = node->netmask = 0;
    node->ipv6 = ipv6;
    node->type = RULE_IPV6;
    node->netaddr = node->netmask = 0;
    node->netmasklen = prefixlen; 
    node->left = node->right = NULL;
    if ( prefixlen == 128 ) {
        node->ipv6start = node->ipv6end = ipv6;
    } else {
        pPrefixStr = getPrefixStr(node->netmasklen);
        ipv6start = (unsigned char *)malloc(16);
        if (!ipv6start){
            goto failure;
        }  
        memset(ipv6start, 0, 16);
        getStartOrEndAddr(ipv6start, pPrefixStr, ipv6, OPT_GET_START_ADDR, node->netmasklen);
        ipv6end = (unsigned char *)malloc(16);
        if (!ipv6end){
            goto failure;
        }
        memset(ipv6end, 255, 16); 
        getStartOrEndAddr(ipv6end, pPrefixStr, ipv6, OPT_GET_END_ADDR, node->netmasklen);
        node->ipv6start = ipv6start;
        node->ipv6end = ipv6end;
        logRecord(pLogCtx, LOG_LEVEL_INFO, "sdssss", __func__, __LINE__, "ipv6 startaddr:", ipv6start, " ipv6 endaddr:", ipv6end);
        if(pPrefixStr) {
            free(pPrefixStr);
        }
    }
    return node;
failure:
    if(pPrefixStr){
        free(pPrefixStr);
    }
    destroyWhiteListTree(node);
    return NULL;
}

static unsigned char *getPrefixStr(int len) {
    unsigned char *prefixStr = NULL;
    int i,j,index = 0;
    prefixStr = (unsigned char *)malloc(16);
    if(!prefixStr) {
        return NULL;
    }
    memset(prefixStr, 0, 16);
    i = len/8;
    j = len%8;
    if(i >= 1 ) {
        while(index < i){
            prefixStr[index++] = 0xff;
        }
        if(j != 0){
            prefixStr[index] = getValue(j);
        }
    }else{
        prefixStr[index] = getValue(j);
    }
    return prefixStr;
}

static void getStartOrEndAddr(unsigned char *pOrigin, unsigned char *prefixAddr, unsigned char *ipv6, int type, int len){
    assert(pOrigin != NULL && prefixAddr != NULL && ipv6 != NULL);
    int index;
    for(index = 0; index < 16; index++) {
        if(type == OPT_GET_START_ADDR) {
            pOrigin[index] = (pOrigin[index] & prefixAddr[index]);
        }else{
            pOrigin[index] = (pOrigin[index] ^ prefixAddr[index]);
        }
    }
    if ( len/8 > 0 ) {
        memcpy(pOrigin, ipv6, (len/8));  
    }else{
        if ( type == OPT_GET_START_ADDR ){
            pOrigin[0] = ( ipv6[0] & getValue(len%8));
        }else{
            pOrigin[0] = (0xff - getValue(len%8)) + ((ipv6[0] & getValue(len%8)) ^ (pOrigin[0] & getValue(len%8)));
        }
    }
}

static int forEachWhiteListRuleTree(struct access_control_elem_s *root, struct access_control_elem_s *pCheckNode) {
    int iRet = RET_ERROR;
    assert(pCheckNode != NULL);
    if( root == NULL ) {
        return iRet;
    }else{
        if(pCheckNode->type == RULE_IPV4) {
            if (root->netaddr ==pCheckNode->netaddr && root->netmask == pCheckNode->netmask) { 
                return RET_OK;
            }   
            if( (pCheckNode->netaddr &  pCheckNode->netmask) < (root->netaddr & root->netmask)) {
                iRet = forEachWhiteListRuleTree(root->left, pCheckNode);
            }else {
                iRet = forEachWhiteListRuleTree(root->right, pCheckNode);
            }

        } else {
            if((memcmp(root->ipv6, pCheckNode->ipv6, 16) == 0 )&&( root->netmasklen == pCheckNode->netmasklen)){
                return  RET_OK;
            } 
            
            if (memcmp(pCheckNode->ipv6, root->ipv6, 16) < 0 ) {
                iRet = forEachWhiteListRuleTree(root->left, pCheckNode);
            }else{
                iRet = forEachWhiteListRuleTree(root->right, pCheckNode);
            }
        }
    }
    return iRet;
}

static struct access_control_elem_s * insertAclRuleNode(struct access_control_elem_s * root, struct access_control_elem_s * curNode){
    assert(curNode != NULL);
    struct access_control_elem_s * node = root;
    if ( node == NULL ){
        node = curNode;
        return node;
    }
    if( curNode->type == RULE_IPV4 ) {
        if((curNode->netaddr & curNode->netmask) < (node->netaddr & node->netmask)){
            node->left = insertAclRuleNode(node->left, curNode);
        }else{    
            node->right = insertAclRuleNode(node->right, curNode);
        }
    }else{
        if((memcmp(curNode->ipv6, node->ipv6, 16)) < 0 ) {
            node->left = insertAclRuleNode(node->left, curNode);
        }else{
            node->right = insertAclRuleNode(node->right, curNode);
        }
    }
    return node;
}

struct access_control_list_s * parseAclRule(struct access_control_list_s *ctx, char *pRule, logContext *pLogCtx) {
    int iRet = RET_ERROR;
    bool isipv6 = isIpv6(pRule);
    struct access_control_elem_s *root = NULL;
    struct access_control_elem_s *ruleNode = NULL;
    unsigned char ipv6[16] = {'\0'};

    if(isipv6){
        root = ctx->ipv6;
        ruleNode = parseIpv6Rule(pRule, pLogCtx);
    }else{
        root = ctx->ipv4;
        ruleNode = parseIpv4Rule(pRule, pLogCtx);
    }
    
    if(ruleNode == NULL){ 
        destroyAclCtx(ctx, pLogCtx);
        return NULL;
    }

    if(( ruleNode->netaddr == 0 && ruleNode->netmasklen == 0 ) || ( ruleNode->netmasklen == 0 && memcmp(ruleNode->ipv6, ipv6, 16) == 0 )){
        ctx->aclEnable = false;
        destroyWhiteListTree(ruleNode);
        return ctx;
    }

    iRet = forEachWhiteListRuleTree(root, ruleNode);
    if( iRet == RET_ERROR ){
        if ( isipv6 ) {
            ctx->ipv6 = insertAclRuleNode(ctx->ipv6, ruleNode);
        } else {
            ctx->ipv4 = insertAclRuleNode(ctx->ipv4, ruleNode);
        }
    }else{
        destroyWhiteListTree(ruleNode);
    }
    return ctx;
}

struct access_control_list_s * parseWhiteListAclRule(webServerContext *pWebContext) {
    configContext *pConfig = pWebContext->pConfig;
    assert(pWebContext != NULL && pConfig != NULL && pConfig->pAclList != NULL);
    char *pAclList = pConfig->pAclList;
    char *pRule = NULL;
    struct access_control_list_s *pCtx = pWebContext->pAclCtx;
    while((pRule = strtok(pAclList, ",")) != NULL ){
        logRecord(pWebContext->pLogCtx, LOG_LEVEL_INFO,  "sdss", __func__, __LINE__, "parse access control node:", pRule);
        pCtx = parseAclRule(pCtx, pRule, pWebContext->pLogCtx);
        if( !pCtx ||pCtx->aclEnable == false ) { 
            break;
        }
        pAclList = NULL;
    }

    if (pCtx&&(pCtx->ipv4 == NULL && pCtx->ipv6 == NULL)) {
        pCtx->aclEnable = false;
    }
    return pCtx;
}

/*@TODO:deinit acl node */
static void destroyWhiteListTree(struct access_control_elem_s *ctx) {
    if( ctx && ctx->type == RULE_IPV4) {
        destroyWhiteListTree(ctx->left);
        destroyWhiteListTree(ctx->right);
        free(ctx);
    }
    if(ctx && ctx->type == RULE_IPV6) {
        if(ctx->netmasklen != 128) {
            free(ctx->ipv6start);
            free(ctx->ipv6end);
        }
        free(ctx->ipv6);
        destroyWhiteListTree(ctx->left);
        destroyWhiteListTree(ctx->right);
        free(ctx);
        ctx = NULL;
    }
}

/*@TODO:deinit acl Context */
void destroyAclCtx(struct access_control_list_s * aclctx, logContext *pLogCtx) {
    if(aclctx){
        destroyWhiteListTree(aclctx->ipv4); 
        destroyWhiteListTree(aclctx->ipv6);
        free(aclctx);
        aclctx = NULL;
    }
    logRecord(pLogCtx, LOG_LEVEL_INFO, "s", "deinit access control list context successfully!");
}

