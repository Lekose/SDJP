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

#pragma region Basic Get First Function
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
		// we can now get the size of the token
		int token_size = token[2].end - token[2].start;

		// not that we have the size we can use our string duplication function (strndup) to create a new string
		// containing only the data contained within that token, token[2] = value so we get a string containing just the value
		char* result = strndup(json_string + token[2].start, token_size);

		// and then we return it
		return result;
	}

	// if you hit this the compare failed so we just return NULL
	// feel free to add some sort of error code system (makes life easier in the long run)
	else
	{
		return NULL;
	}

}
#pragma endregion


// First we want to return the corresponding value of some of these fields, to do this we are going to create a struct called WhoAmI
// this contains entries for each value we want to retrieve, not sure if you guys have come across structs yet, if not a good guide:
// https://www.geeksforgeeks.org/structures-c/ which covers the basics
typedef struct
{
	char* FirstName;
	char* MiddleName;
	char* LastName;
	char* Occupation;
	int ErrorCode;
} WhoAmI;

WhoAmI basic_nested(char* json_string)
{
	// First we create our empty struct we are going to use to hold the values
	WhoAmI whoami;

	// We create our parser object
	sdjp_parser parser;

	// We create the number of tokens we want (128 seems like a solid number)
	// TODO: maybe have this passed as a function parameter
	sdjptok_t token[128];

	// Initalizes our parser object
	sdjp_init(&parser);

	// So this is similar to the basic_get_first version
	// For number of tokens I use sizeof(token) / sizeof(token[0])
	// This just evaluates to 2048/16 = 128
	// it saves us having to update the code in two places if the max number of tokens changes
	int result = sdjp_parse(&parser, json_string, strlen(json_string), token, sizeof(token) / sizeof(token[0]));

	// If result is less than 0 an error code was returned, we print it
	if (result < 0)
	{
		printf("Failed to tokenize JSON: %d\n", result);
		// Set our error code
		whoami.ErrorCode = 1;
		// Return from function
		return whoami;
	}

	// This checks if the first entry in the token array is the expected type SDJP_OBJECT, if not then something has likely gone wrong with our tokenizer
	if (result == 0 || token[0].type != SDJP_OBJECT)
	{
		printf("first entry in token array not a SDJP_OBJECT, something went wrong");
		// Set our error code
		whoami.ErrorCode = 1;
		// Return from function
		return whoami;
	}

	// Now we get into our loop, the logic is similar to our first implementation except this time we check for a bunch of predefined names
	// If we find one then we append it to the appropriate field inside our WhoAmI structure
	for (int i = 1; i < result; i++)
	{
		if (json_compare(json_string, &token[i], "FirstName") == 0)
		{
			// copies our string using the values provided by our token
			// we use i+1 as i is the token for our name and +1 from that is the associated value
			whoami.FirstName = strndup(json_string + token[i+1].start, token[i+1].end - token[i+1].start);
			i++;
		}

		else if (json_compare(json_string, &token[i], "MiddleName") == 0)
		{
			whoami.MiddleName = strndup(json_string + token[i + 1].start, token[i + 1].end - token[i + 1].start);
			i++;
		}

		else if (json_compare(json_string, &token[i], "LastName") == 0)
		{
			whoami.LastName = strndup(json_string + token[i + 1].start, token[i + 1].end - token[i + 1].start);
			i++;
		}

		else if (json_compare(json_string, &token[i], "Occupation") == 0)
		{
			whoami.Occupation = strndup(json_string + token[i + 1].start, token[i + 1].end - token[i + 1].start);
			i++;
		}
	}

	// if we get here then something is being returned so just set the error code to 0 (success)
	whoami.ErrorCode = 0;

	// return
	return whoami;

}



int main()
{	
	// Calls our basic implementation where we know the name of our JSON field beforehand (in this case its Hello)
	// see function for more details
	char* json_test = "{\"Hello\":\"World\"}";
	char* bgf_result = basic_get_first(json_test, "Hello");


	// A bit more oomfy example, here we have a raw JSON object with multiple root fields
	// What we are going to do is loop through each root field and check if the name is something we expect
	// if it is then we are going to process it. Again with this example we assume we know the name ahead of time.

	// Here is our example JSON, we have 4 root fields here FirstName, MiddleName, LastName and Occupation
	char* json_test2 = "{\"FirstName\":\"James\", \"MiddleName\":\"Henry\", \"LastName\":\"Chadwick\", \"Occupation\":\"Pro Nerd\"}";
	
	// call our function
	WhoAmI test2 = basic_nested(json_test2);

	// print the results
	printf("FirstName = %s\n", test2.FirstName);
	printf("MiddleName = %s\n", test2.MiddleName);
	printf("LastName = %s\n", test2.LastName);
	printf("Occupation = %s\n", test2.Occupation);

	// TODO for fun try changing a field in our json string to an unexpected one, what happens?
	// How could you solve that problem? HINT check the test2 struct field you removed before you try to print it

	return 0;
}