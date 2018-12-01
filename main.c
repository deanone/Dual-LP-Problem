#include <stdio.h>
#include <string.h>
#include <stdlib.h>  

typedef enum { false, true } bool;
#define maxLineSize 255

size_t countCharOccurences(char *s, char c)
{
	size_t count = 0;
	for (size_t i = 0; s[i]; ++i) count += (s[i] == c);
	return count;
}

void sliceStr(const char *s, char * token, size_t start, size_t end)
{
	size_t j = 0;
	for (size_t i = start; i <= end; ++i)
		token[j++] = s[i];
	token[j] = 0;	// manual null termination
}

size_t countlines(const char *filename)
{
	// count the number of lines in the file                                    
	FILE *file = fopen(filename, "r");
	int ch = 0;
	size_t lines = 0;

	if (file)
	{
		lines++;
		while ((ch = fgetc(file)) != EOF)
		{
			if (ch == '\n')
				lines++;
		}
		fclose(file);
	}
	return lines;
}

size_t strlstchar(const char *s, const char c)
{
	char *cptr = strrchr(s, c);
	return cptr - s;
}

void transpose(double **A, int nrows, int ncols, double **At)
{
	for (int j = 0; j < ncols; ++j)
	{
		double *AtRow = (double *)malloc(nrows * sizeof(double));
		for (int i = 0; i < nrows; ++i)
		{
			AtRow[i] = A[i][j];
		}
		At[j] = AtRow;
	}
}

int main(int argc, char **argv)
{
	// The file 'lp1.txt' should be located in the same folder as the executable
	char *path = NULL;
	size_t pathSize = strlstchar(argv[0], '\\');
	path = (char *)malloc((pathSize) * sizeof(char));
	sliceStr(argv[0], path, 0, pathSize - 1);

	char *inFilename = (char *)malloc((strlen(path) + 8) * sizeof(char));
	char *outFilename = (char *)malloc((strlen(path) + 8) * sizeof(char));

	strncpy(inFilename, path, (strlen(path) + 8));
	strcat(inFilename, "\\lp1.txt");

	strncpy(outFilename, path, (strlen(path) + 8));
	strcat(outFilename, "\\lp2.txt");

	free(path);

	size_t numOfArows = countlines(inFilename) - 1;

	// Initialize A matrix
	double **A;
	A = (double **)malloc(numOfArows * sizeof(double *));
	size_t Arow_id = 0;
	size_t numOfAcols = 0;

	// Initialize b vector
	double *b = NULL;
	b = (double *)malloc(numOfArows * sizeof(double));
	size_t b_id = 0;

	// Initialize Eqin vector
	int *Eqin = NULL;
	Eqin = (int *)malloc(numOfArows * sizeof(int));
	size_t Eqin_id = 0;

	// objective function's coefficients
	double *c = NULL;
	size_t numOfCoeffs = 0;

	// Optimization problem type
	int MinMax = 0;

	// subject to
	char *st = NULL;

	// delimiters
	char delimiter1 = ' ';
	char delimiter2 = '+';

	FILE * file;

	file = fopen(inFilename, "r");
	free(inFilename);
	bool firstLine = true;
	bool secondLine = true;

	char *line = NULL;
	line = (char *)malloc(maxLineSize * sizeof(char));
	if (file)
	{
		while (fgets(line, maxLineSize, (FILE*)file))
		{
			if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
			// Split the objective function line based on the ' ' delimiter
			char* token;
			char** tokens;
			size_t numOfTokens = countCharOccurences(line, delimiter1) + 1;
			tokens = (char **)malloc(numOfTokens * sizeof(char*));

			size_t len = strlen(line);
			size_t token_id, start, end;
			token_id = start = end = 0;
			for (size_t i = 0; i < len; ++i)
			{
				if (line[i] == delimiter1)
				{
					end = i;
					token = (char *)malloc((end - start + 1) * sizeof(char));
					sliceStr(line, token, start, end - 1);
					tokens[token_id] = token;
					token_id++;
					start = end + 1;
				}
			}
			// Read the last slice of the line from the last occurence of ' '
			// up to the end of line
			end = len;
			token = (char *)malloc((end - start + 1) * sizeof(char));
			sliceStr(line, token, start, end - 1);
			tokens[token_id] = token;

			if (firstLine)
			{
				firstLine = false;

				// Get the number of c coefficients by counting the number of '+' characters
				// number of c coefficients = (number of '+' characters) + 1
				numOfCoeffs = countCharOccurences(line, delimiter2) + 1;
				c = malloc(numOfCoeffs * sizeof(double));

				// Set the type of the optimization problem
				MinMax = (strcmp("min", tokens[0]) == 0) ? -1 : 1;

				// Convert the objective function's coefficients
				// from char * to double
				size_t coeff_id = 0;
				for (size_t i = 1; i < numOfTokens; ++i)
				{
					if (strlen(tokens[i]) > 1)
					{
						char *rest;
						double coeff_d = strtod(tokens[i], &rest);
						c[coeff_id] = coeff_d;
						coeff_id++;
					}
				}

				// Clear memory
				//for (size_t i = 0; i < numOfTokens; i++)
				//	free(tokens[i]);
				free(tokens);
				continue;
			}
			else
			{
				size_t startOfTokens = -1;
				if (secondLine)
				{
					secondLine = false;
					// Parse the type of optimization problem
					st = (char *)malloc((strlen(tokens[0]) + 1) * sizeof(char));
					strcpy(st, tokens[0]);
					startOfTokens = 1;
				}
				else
				{
					startOfTokens = 0;
				}
				// Parse the coefficients of the A matrix
				size_t coeff_id = 0;
				double *Arow = NULL;
				size_t numOfCoeffs = countCharOccurences(line, delimiter2) + 1;
				numOfAcols = numOfCoeffs;
				Arow = (double *)malloc(numOfCoeffs * sizeof(double));
				for (size_t i = startOfTokens; i < (numOfTokens - 2); ++i)
				{
					if (strlen(tokens[i]) > 1)	// if token is not '+', '=', '>=' or '<='
					{
						char *rest = NULL;
						double coeff_d = strtod(tokens[i], &rest);
						Arow[coeff_id] = coeff_d;
						coeff_id++;
					}
				}
				A[Arow_id] = Arow;
				Arow_id++;

				// Parse '=', '>=' or '<=' symbol
				size_t symbol;
				if (strcmp(tokens[numOfTokens - 2], "=") == 0)
				{
					symbol = 0;
				}
				else if (strcmp(tokens[numOfTokens - 2], ">=") == 0)
				{
					symbol = 1;
				}
				else if (strcmp(tokens[numOfTokens - 2], "<=") == 0)
				{
					symbol = -1;
				}

				Eqin[Eqin_id] = symbol;
				Eqin_id++;

				// Parse the coefficients of b vector
				char *rest;
				double coeff_d = strtod(tokens[numOfTokens - 1], &rest);
				b[b_id] = coeff_d;
				b_id++;
			}
		}
		fclose(file);
	}

	// Find the dual structures
	double **At;
	At = (double **)malloc(numOfAcols * sizeof(double *));
	transpose(A, numOfArows, numOfAcols, At);
	free(A);

	int maxNumOfCoeffs = (numOfCoeffs >= numOfArows) ? numOfCoeffs : numOfArows;
	
	double *b_dual = NULL;
	b_dual = (double *)malloc(maxNumOfCoeffs * sizeof(double));
	for (size_t i = 0; i < maxNumOfCoeffs; ++i)
	{
		b_dual[i] = 0.0;
	}
	for (size_t i = 0; i < numOfArows; ++i)
	{
		b_dual[i] = b[i];
	}
	free(b);

	double *c_dual = NULL;
	c_dual = (double *)malloc(maxNumOfCoeffs * sizeof(double));
	for (size_t i = 0; i < maxNumOfCoeffs; ++i)
	{
		c_dual[i] = 0.0;
	}
	for (size_t i = 0; i < numOfCoeffs; ++i)
	{
		c_dual[i] = c[i];
	}
	free(c);

	// Swap values of b (dual) and c (dual) vectors
	for (size_t i = 0; i < maxNumOfCoeffs; ++i)
	{
		double temp = b_dual[i];
		b_dual[i] = c_dual[i];
		c_dual[i] = temp;
	}

	file = fopen(outFilename, "w");
	free(outFilename);
	if (MinMax == -1)	// min
	{
		fprintf(file, "max ");
		fprintf(file, "c_dual'w\n");
		fprintf(file, "%s ", st);
		fprintf(file, "A_dualw@b_dual\n");
		for (size_t i = 0; i < numOfArows; ++i)
		{
			if (Eqin[i] == -1)
			{
				fprintf(file, "w%d <= 0, ", i + 1);
			}
			else if (Eqin == 1)
			{
				fprintf(file, "w%d >= 0, ", i + 1);
			}
			else
			{
				fprintf(file, "w%d free, ", i + 1);
			}

			Eqin[i] = -1;
		}
	}
	else    // max
	{
		fprintf(file, "min ");
		fprintf(file, "c_dual'w\n");
		fprintf(file, "%s ", st);
		fprintf(file, "A_dualw@b_dual\n");
		for (size_t i = 0; i < numOfArows; ++i)
		{
			if (Eqin[i] == -1)
			{
				fprintf(file, "w%d >= 0, ", i + 1);
			}
			else if (Eqin == 1)
			{
				fprintf(file, "w%d <= 0, ", i + 1);
			}
			else
			{
				fprintf(file, "w%d free, ", i + 1);
			}

			Eqin[i] = 1;
		}
	}

	fprintf(file, "where:\n\n");

	fprintf(file, "A_dual = \n");
	for (size_t j = 0; j < numOfAcols; ++j)
	{
		for (size_t i = 0; i < numOfArows; ++i)
		{
			if (i != (numOfArows - 1))
				fprintf(file, "%.2f ", At[j][i]);
			else
				fprintf(file, "%.2f", At[j][i]);
		}
		fprintf(file, "\n");
	}

	fprintf(file, "\n");

	fprintf(file, "b_dual = \n");
	for (size_t i = 0; i < maxNumOfCoeffs; ++i)
	{
		fprintf(file, "%.2f\n", b_dual[i]);
	}

	fprintf(file, "\n");

	fprintf(file, "@ = \n");
	for (size_t i = 0; i < numOfArows; ++i)
	{
		fprintf(file, "%d\n", Eqin[i]);
	}

	fprintf(file, "\n");

	fprintf(file, "c_dual = \n");
	for (size_t i = 0; i < maxNumOfCoeffs; ++i)
	{
		fprintf(file, "%.2f\n", c_dual[i]);
	}

	fclose(file);

	// Clear memory
	free(At);
	free(b_dual);
	free(c_dual);
	free(Eqin);
	free(line);
	printf("\n");
	return 0;
}