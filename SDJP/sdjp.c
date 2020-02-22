#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdjp.h"


/*
	json_compare is a function for checking that a particular token value matches an expected result

	INPUTS:
	    const char* json = pointer to raw json string
		sdjptok_t* tok = our JSON token desciptor
		const char* s = pointeer to the value we want to check is at the token
		
	OUTPUTS:
		int 0 if compare succeeds, that is to say value at token is equivilent to that passed in s
		-1 if the compare fails

	NOTES:
		- Could probably do this function as a BOOL, pretty much is already
		- strncmp defines maxCount as a size_t, not really an issue but generates annoying VS compiler warning
		- Maybe move this function into the core SDJP library?
*/
static int json_compare(const char* json, sdjptok_t* tok, const char* s) 
{
	if (tok->type == SDJP_STRING && (int)strlen(s) == tok->end - tok->start &&
		strncmp(json + tok->start, s, tok->end - tok->start) == 0) 
	{
		return 0;
	}
	return -1;
}


char* strndup(char* str, size_t chars)
{
	char* buffer;
	size_t n;

	buffer = (char*)malloc(chars + 1);
	if (buffer)
	{
		for (n = 0; ((n < chars) && (str[n] != '\0')); n++)
		{
			buffer[n] = str[n];
		}
		buffer[n] = '\0';
	}

	return buffer;
}

int main()
{

	char* json_test = "{\"Hello\":\"World\"}";


	sdjp_parser p;

	sdjptok_t t[128];

	sdjp_init(&p);

	int r = sdjp_parse(&p, json_test, strlen(json_test), t, 128);


	if (json_compare(json_test, &t[1], "Hello") == 0) 
	{
		/* We may use strndup() to fetch string value */
		printf("- Hello: %.*s\n", t[1 + 1].end - t[1 + 1].start,
			json_test + t[1 + 1].start);


		int token_size = t[2].end - t[2].start;

		char* result = strndup(json_test + t[2].start, token_size);
		printf("Result is: %s",result);

	}

	

	return 0;
}