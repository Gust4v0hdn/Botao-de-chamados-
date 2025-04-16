#include <stdio.h>
int main () {

int flamengo,palmeiras;

printf ("quantos gol do flamengo\n");
scanf ("%d",&flamengo);
printf ("quantos gol do palmeiras\n");
scanf ("%d",&palmeiras);
if (flamengo>palmeiras){
printf ("flamengo venceu\n");
}
else if (flamengo<palmeiras) {
printf ("palmeiras venceu\n");
}
else {
printf ("empate\n");
}
return 0;
}