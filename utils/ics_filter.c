/** 
 * @TODO: 1. filter only the 1st week events 
 * @TODO: 2. add automatic generation of repeating events by adding:  RRULE:FREQ=WEEKLY;UNTIL=20241219T090000
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
void putLine(char *buffer, FILE *f) {
	fputs(buffer, f);
	fputc('\n', f);	//print EOLN, because fputs does not print EOLN!
}
int main(int argc, char *argv[]) {
	FILE *f = stdin, *of=stdout;
	char *eventString = "AUIUI/AK";
	if (argc < 3) {
		fprintf(stderr, "Cmd line missing arguments: FILE.ICS EVENT_STRING [OUTPUT_FILE.ICS]."
				"I am using 'stdin' %s 'stdout'.\n"
				"The EVENT_STRING is any string which occurs inside the EVENT section of the ICS file\n"
				"(EVENT section starts by 'BEGIN:VEVENT' and finishes by 'END:VEVENT' line).\n"
				"If the EVENT_STRING is found inside the EVEN section, the whole EVENT (its section) is copied into the output file.\n"
				"Otherwise, the EVENT is skipped.\n", eventString);
	}
	else {
		f = fopen(argv[1], "r");
		if (f == NULL) {
			fprintf(stderr, "File %s not found!\n", argv[1]);
			exit(EXIT_FAILURE);
		}
		eventString = argv[2];
		if (argc>3) {
			of = fopen(argv[3], "w");
			if (of == NULL) {
				fprintf(stderr, "File %s not found!\n", argv[3]);
				exit(EXIT_FAILURE);
			}
		}
	}
	char eventLines[50][255];
	char * buffer=eventLines[0];
	enum {
		START, EVENTS, EVENT_FOUND
	} state = START;
	int linenr = 0;
	while (fgets(buffer, 255, f) != NULL) {
		size_t len = strcspn(buffer, "\r\n");
		buffer[len] = 0;		//crop the string at the first occurence of \r or \n by

		switch (state) {
		case START:			// start of the ICS file = copy all the header lines to the output
			if (strstr(buffer, "BEGIN:VEVENT") != NULL) {
				state = EVENTS;	//after first VEVENT, start filtering the calendar events
				buffer=eventLines[++linenr]; //point the buffer to the next empty line in array
			}
			else putLine(buffer, of);
			break;
		case EVENTS:
			/**
			 * Here we store all lines of all events into an array for processing later,
			 * because the event name is typically on linenr==5 which looks like this:
			 * SUMMARY:AUIUI/AK1PM - Přednáška
			 */
			if (strstr(buffer, eventString) != NULL) {
				state=EVENT_FOUND;
				buffer=eventLines[++linenr]; //point the buffer to the next empty line in array
			} else 	if (strstr(buffer, "END:VEVENT") != NULL) {	//this event will not be printed...
				linenr = 0;
				buffer=eventLines[0];
				state=START;
			} else {
				buffer=eventLines[++linenr]; //point the buffer to the next empty line in array
			}
			break;
		case EVENT_FOUND:// here we store all lines of all events into an array for later processing
			if (strstr(buffer, "END:VEVENT") != NULL) {
				for (int i = 0; i <= linenr; i++) {
					putLine(eventLines[i], of);
				}
				linenr = 0;
				buffer=eventLines[0];
				state=START;
			} else {
				buffer=eventLines[++linenr]; //point the buffer to the next empty line in array
			}
			break;
		}
		if (linenr>=50) {
			fputs("ICS file syntax error causing buffer array overflow.\n", stderr);
			exit(EXIT_FAILURE);
		}
	}
	fputs("Output ICS generated successfully.\n", stderr);
	return EXIT_SUCCESS;
}
