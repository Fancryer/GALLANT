#include<iostream>
#include"Lexer.h"

typedef struct
{
	const char* start;
	const char* current;
	int line;
}Scanner;

Scanner scanner;

void InitScanner(const char* Source)
{
	scanner.start=Source;
	scanner.current=Source;
	scanner.line=1;
}
static bool IsAlpha(char c) { return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }
static bool IsDigit(char c) { return c>='0'&&c<='9'; }
static bool IsAtEnd() { return *scanner.current=='\0'; }
static char Advance() { ++scanner.current;return scanner.current[-1]; }
static char Peek() { return *scanner.current; }
static char PeekNext() { if(IsAtEnd())return '\0';return scanner.current[1]; }
static bool Match(char expected) { if(IsAtEnd()||*scanner.current!=expected) return false;++scanner.current;return true; }
static Token MakeToken(TokenType type) { return {type,scanner.start,(int)(scanner.current-scanner.start),scanner.line}; }
static Token ErrorToken(const char* message) { return {TOKEN_ERROR,message,(int)strlen(message),scanner.line}; }
static void SkipWhitespace()
{
	for(;;)
	{
		char c=Peek();
		switch(c)
		{
			case ' ':
			case '\r':
			case '\t':
				Advance();
				break;
			case '\n':
				scanner.line++;
				Advance();
				break;
			case '/':
				if(PeekNext()=='/')
				{
					while(Peek()!='\n'&&!IsAtEnd())Advance();//A comment goes until the end of the line.
				}
				else return;
				break;
			default:return;
		}
	}
}

static TokenType CheckKeyword(int start,int length,const char* rest,TokenType type)
{
	return(scanner.current-scanner.start==start+length&&memcmp(scanner.start+start,rest,length)==0)?type:TOKEN_IDENTIFIER;
}

static TokenType IdentifierType()
{
	switch(scanner.start[0])
	{
		case 'a': return CheckKeyword(1,2,"nd",TOKEN_AND);
		case 'c': return CheckKeyword(1,4,"lass",TOKEN_CLASS);
		case 'e': return CheckKeyword(1,3,"lse",TOKEN_ELSE);
		case 'f':
			if(scanner.current-scanner.start>1)
			{
				switch(scanner.start[1])
				{
					case 'a': return CheckKeyword(2,3,"lse",TOKEN_FALSE);
					case 'o': return CheckKeyword(2,1,"r",TOKEN_FOR);
					case 'u': return CheckKeyword(2,1,"n",TOKEN_FUN);
				}
			}
			break;
		case 'i': return CheckKeyword(1,1,"f",TOKEN_IF);
		case 'n': return CheckKeyword(1,2,"il",TOKEN_NIL);
		case 'o': return CheckKeyword(1,1,"r",TOKEN_OR);
		case 'p': return CheckKeyword(1,4,"rint",TOKEN_PRINT);
		case 'r': return CheckKeyword(1,5,"eturn",TOKEN_RETURN);
		case 's': return CheckKeyword(1,4,"uper",TOKEN_SUPER);
		case 't':
			if(scanner.current-scanner.start>1)
			{
				switch(scanner.start[1])
				{
					case 'h': return CheckKeyword(2,2,"Is",TOKEN_THIS);
					case 'r': return CheckKeyword(2,2,"ue",TOKEN_TRUE);
				}
			}
			break;
		case 'v': return CheckKeyword(1,2,"ar",TOKEN_VAR);
		case 'w': return CheckKeyword(1,4,"hile",TOKEN_WHILE);
	}
	return TOKEN_IDENTIFIER;
}

static Token identifier()
{
	while(IsAlpha(Peek())||IsDigit(Peek()))Advance();
	return MakeToken(IdentifierType());
}

static Token number()
{
	while(IsDigit(Peek()))Advance();

	// Look for a fractional part.
	if(Peek()=='.'&&IsDigit(PeekNext()))
	{
		// Consume the ".".
		Advance();

		while(IsDigit(Peek()))Advance();
	}

	return MakeToken(TOKEN_NUMBER);
}

static Token string()
{
	while(Peek()!='"'&&!IsAtEnd())
	{
		if(Peek()=='\n') scanner.line++;
		Advance();
	}

	if(IsAtEnd()) return ErrorToken("Unterminated string.");

	// The closing quote.
	Advance();
	return MakeToken(TOKEN_STRING);
}

Token scanToken()
{
	SkipWhitespace();
	scanner.start=scanner.current;

	if(IsAtEnd()) return MakeToken(TOKEN_EOF);

	char c=Advance();
	if(IsAlpha(c)) return identifier();
	if(IsDigit(c)) return number();
	switch(c)
	{
		case '(': return MakeToken(TOKEN_LEFT_PAREN);
		case ')': return MakeToken(TOKEN_RIGHT_PAREN);
		case '{': return MakeToken(TOKEN_LEFT_BRACE);
		case '}': return MakeToken(TOKEN_RIGHT_BRACE);
		case ';': return MakeToken(TOKEN_SEMICOLON);
		case ',': return MakeToken(TOKEN_COMMA);
		case '.': return MakeToken(TOKEN_DOT);
		case '-': return MakeToken(TOKEN_MINUS);
		case '+': return MakeToken(TOKEN_PLUS);
		case '/': return MakeToken(TOKEN_SLASH);
		case '*': return MakeToken(TOKEN_STAR);
		case '!':
			return MakeToken(
				Match('=')?TOKEN_BANG_EQUAL:TOKEN_BANG);
		case '=':
			return MakeToken(
				Match('=')?TOKEN_EQUAL_EQUAL:TOKEN_EQUAL);
		case '<':
			return MakeToken(
				Match('=')?TOKEN_LESS_EQUAL:TOKEN_LESS);
		case '>':
			return MakeToken(
				Match('=')?TOKEN_GREATER_EQUAL:TOKEN_GREATER);
		case '"': return string();
	}

	return ErrorToken("Unexpected character.");
}