// use DIAG when possible

@i@
@@

#include <tap/basic.h>

@depends on i@
identifier res;
expression E;
@@

(
res &= DIAG(E);
|
-res &= E;
+res &= DIAG(E);
)
