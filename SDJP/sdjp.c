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

/*
	strndup is a string duplication function, its a fairly common thing on Linux systems but there is no Windows version
	...so i made one. Its pretty straight forward, pass it a string and the string size and it will return a copy of that
	string (with a null byte at the end)

	INPUTS
		char* str = pointer to the string you want to copy
		size_t chars = size of the string you want to copy

	OUTPUTS
		char* buffer = pointer to the new copied string (NULL terminated)

	NOTES
		- Another one that *could* be included in the SDJP core, kinda 50/50 on it though as its more a utility function than anything
		- VS compiler gives me a potential overflow warning when appending the NULL byte, fairly sure this is bullshit but might be worth double checking
*/
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

/*
	basic_get_first is a function that will basically pull the value from a defined name
	in other words if you know your response from your server will be:
		{ "Result" : "some string" }
	you could use this function to extract the value (some string) because your name (Result) is constant and known beforehand
	While this is more limiting the code to do it is much smaller

	INPUTS:
		char* json_string = your raw json_string from the server
		char* value = the name that is known ahead of time

	OUTPUTS:
		char* result which will contain the value associated with the passed name

	NOTES:
		This is the most basic way to accomplish the functionality you want but does mean that the name element will have to be
		known beforehand, as you control the server you could define this as a interface standard.
*/
char* basic_get_first(char* json_string, char* name)
{
	// We create our parser object
	sdjp_parser parser;
	// We create the number of tokens we want
	//    token[0] = reserver (SDJP uses it internally)
	//    token[1] = the name of your JSON field
	//    token[2] = the value associated with that name (which is returned)
	sdjptok_t token[3];
	
	// Initializes our parser with some default values
	sdjp_init(&parser);

	// sdjp_parse does the actual tokenizing of the JSON string
	//    INPUTS:
	//        &parser = our initalized parser object
	//        json_string = our raw JSON string
	//        strlen(json_string) = length of json string
	//	      token = our token array
	//        3 = number of tokens we want to use, this can be different from the total number of assigned tokens (in this case its the same)
	//    OUTPUT:
	//        result = if sucessful this will return the number of assigned tokens (should be 3 using this example)
	//	      if an error occurs it will return a minus number which will match a value in the sdjperr enum
	int result = sdjp_parse(&parser, json_string, strlen(json_string), token, 3);

	// So at this point our token array should look like this:
	//     token[0] = reserved (used internally by sdjp
	//     token[1] = The name of the json name:value pair passed in the raw JSON string
	//     token[2] = the value of the json name:value pair passed in the raw JSON string
	// we want to check if the name is what we expect (this is known ahead of time while the value is not) so we can use our json_compare function

	// INPUTS:
	//    json_string = our raw JSON string
	//    &token[1] = reference to our 2nd entry in our token array (the name)
	//    name = the known beforehand name that we passed to the basic_get_first function

	// OUTPUTS:
	//    0 = name matches the value at the referenced token (it worked)
	//    1 = name does not match (RIP sweet prince)
	if (json_compare(json_string, &token[1], name) == 0)
	{
		// each token object has a start and end
		// these are integers we define as part of the sdjptok_t object
		// .start = the position in our JSON string that is the first character of the tokenized object
		// .end = the position in our JSON string that is the last character of the tokenized object
		int token_size = token[2].end - token[2].start;

		char* result = strndup(json_string + token[2].start, token_size);
		return result;
	}
	else
	{
		return NULL;
	}

}


int main()
{



	char* json_test = "{\"Hello\":\"World\"}";

	char* bgf_result = basic_get_first(json_test, "Hello");

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