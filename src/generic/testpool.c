#include "interpool.h"
#include "mod_websh.h"

int main(int argc, char **argv)
{

    WebInterp *a, *b1, *b2;
    websh_server_conf *c =
	(websh_server_conf *) malloc(sizeof(websh_server_conf));

    c->scriptName = NULL;
    c->mainInterp = NULL;
    c->mainInterpLock = NULL;
    c->webshPool = NULL;
    c->webshPoolLock = NULL;

    initPool(c);

    a = poolGetWebInterp(c, "a", "/home/andrej/.cshrc", 0);
    b1 = poolGetWebInterp(c, "b", "/home/andrej/.cshrc", 0);
    b2 = poolGetWebInterp(c, "b", "/home/andrej/.cshrc", 0);
    poolReleaseWebInterp(a);
    poolReleaseWebInterp(b1);
    poolReleaseWebInterp(b2);

    destroyPool(c);

    free(c);

    return 0;

}
