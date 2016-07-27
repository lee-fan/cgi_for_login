#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>

void createUUID(char *uuid);
int main()
{
	char uuid[1024] = {0};
	createUUID(uuid);
	printf("uuid=%s\n",uuid);
	printf("len=%d\n",strlen(uuid));

	return 0;
}
void createUUID(char *uuid)
{
	int i;
	uuid_t uu;
	char tmp[10];

	uuid_generate(uu);
	for(i=0; i<16; i++)
	{
		sprintf(tmp,"%x",uu[i]);
		strcat(uuid,tmp);
		memset(tmp,0,sizeof(tmp));
	}
}
