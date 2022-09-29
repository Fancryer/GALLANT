#include<iostream>
#include<stack>
#include<vector>
#include<string>
#include<limits>
#include<format>

bool NOR(bool a,bool b) { return!(a||b); }
bool NOT(bool a) { return NOR(a,a); }
bool AND(bool a,bool b) { return NOR(NOR(a,a),NOR(b,b)); }
bool OR(bool a,bool b) { return NOR(NOR(a,b),NOR(a,b)); }
bool XOR(bool a,bool b) { return AND(OR(a,b),NOT(AND(a,b))); }
bool EQL(bool a,bool b) { return OR(AND(NOT(a),NOT(b)),AND(a,b)); }

namespace GALL
{
	typedef unsigned int uint;
	using namespace std;

	class Interpreter
	{

	private:
		stack<uint> CalcStack;
		uint Variables[26]={};
		stack<char> VarStack;
		uint Consts[26]=
		{
			10,20,30,40,50,60,70,80,90,100,200,
			300,400,500,600,700,800,900,1000,
			2000,3000,4000,5000,6000,7000,8000
		};
		uint R=0;	//Temp value
		bool bIsUsingCalcStack=true;
		bool bIsInLoop=false;
		bool bIsInMacro=false;
		size_t RepeatIndex;

		/*
		inline static uint64_t ShaveToken=StringToCaseVal("Shave");
		inline static uint64_t AddToken=StringToCaseVal("Add");
		inline static uint64_t SubtractToken=StringToCaseVal("Subtract");
		inline static uint64_t MultiplyToken=StringToCaseVal("Multiply");
		inline static uint64_t DivideToken=StringToCaseVal("Divide");
		inline static uint64_t PowerToken=StringToCaseVal("Power");
		inline static uint64_t ConcatToken=StringToCaseVal("Concat");
		*/

		typedef enum
		{
			//Math
			T_ADD,			// +
			T_SUB,			// -
			T_MULT,			// *
			T_DIV,			// /
			T_CAT,			// _
			T_POW,			// ^
			//Logical
			T_LESS,			// <
			T_EQUAL,		// =
			T_GREATER,		// >
			T_NOR,			// |

			T_CONST,		// A-Z
			T_TOP,			// ? - print stack top as integer
			T_SYM,			// $ - print stack top as char
			T_WHITESPACE,		// , - print whitespace
			T_NEWLINE,		// . - print newline
			T_VARIABLE,
			T_ASSIGN,		// : - assign stack top to another stack top, then pop it
			//T_READ,		// , - read symbol from console input
			T_SHAVE,		// ~ - pop stack top
			T_EXIT,			// ! - finish program	
			T_SWITCH,		// # - switch working stack
			T_CLEAR,		// % - clear stack
			T_REVERSE,		// ; - reverse stack
			T_REPEAT,		// [ - loop start
			T_UNTIL,		// ] - loop end
			T_MOPEN,		// ( - macro start
			T_MCLOSE,		// ) - macro end
			T_UNDEFINED
		}TokenType;

		typedef enum
		{
			E_NONE,
			E_NOP,			//Not operator
			E_EXIT,
			E_OVERFLOW,
			E_NEGATIVE,
			E_NOTSYMBOL,
			E_ZERODIVISION,
			E_EmptyStackAccessing,
			E_EmptyStackShaving,
			E_WRONGSTACK,		//Wrong stack is using
			E_NESTEDLOOP,
			E_NOTINLOOP
			//E_NID			//Not ID
		}ErrorType;

		const bool IsDigit(char c) { return c>='0'&&c<='9'; }
		const bool IsAlpha(char c) { return c>='A'&&c<='Z'; }
		const bool IsNumber(char c) { return IsAlpha(c)||IsDigit(c); }
		const bool IsVariable(char c) { return c>='a'&&c<='z'; }
		const bool AreStacksFilled() { return !(CalcStack.empty()||VarStack.empty()); }

		/*
		* DEPRECATED
		const bool IsOperator(char c) { return c=='+'||c=='-'||c=='*'||c=='/'||c=='_'||c=='^'||c=='?'||c=='~'; }
		*/

		//Ready to work with any stack
		ErrorType PrepareStackToMath(uint& A,uint& B)
		{
			if(bIsUsingCalcStack)
			{
				if(CalcStack.size()<2)return E_EmptyStackAccessing;
				GetCalcTop(A);
				GetCalcTop(B);
			}
			else
			{
				if(VarStack.size()<2)return E_EmptyStackAccessing;
				GetVarTop(A);
				GetVarTop(B);
			}
			return E_NONE;
		}

		void GetCalcTop(uint& Temp) { Temp=CalcStack.top();CalcStack.pop(); }
		void GetVarTop(uint& Temp) { Temp=Variables[VarStack.top()-'a'];Variables[VarStack.top()-'a']=0;VarStack.pop(); }

		uint TokenAsNumber(char c) { return IsDigit(c)?(uint)(c-'0'):Consts[c-'A']; }
		uint VariableAsNumber(char Var) { return Variables[Var-'a']; }

		TokenType GetType(char Token)
		{
			switch(Token)
			{
				/*
				* Valid tokens:
					+ - * / _ < = > ` [ ] ( ) ? $ ~ : ! # % ;
				* Invalid tokens:
					\ " & ' . , @ ^ { | }
				*/
				//Math
				case'+':return T_ADD;
				case'-':return T_SUB;
				case'*':return T_MULT;
				case'/':return T_DIV;
				case'_':return T_CAT;
				//Logical
				case'<':return T_LESS;
				case'=':return T_EQUAL;
				case'>':return T_GREATER;
				case'`':return T_NOR;
				//Loop
				case'[':return T_REPEAT;
				case']':return T_UNTIL;
				//Macro
				case'(':return T_MOPEN;
				case')':return T_MCLOSE;
				//Stack top oparations
				case'?':return T_TOP;			//Print stack top value as integer
				case'$':return T_SYM;			//Print stack top value as char
				case'~':return T_SHAVE;			//Pop stack top
				case':':return T_ASSIGN;		//Assign top of this stack top to another stack top and pop this stack top
				//Service
				case'!':return T_EXIT;			//Finish program
				case'#':return T_SWITCH;
				case'%':return T_CLEAR;
				case';':return T_REVERSE;
			}
			if(IsNumber(Token))return T_CONST;		//Push const to calc stack
			if(IsVariable(Token))return T_VARIABLE;		//Push variable to var stack
			return T_UNDEFINED;
		}

		const string TypeToString(TokenType Type)
		{
			const string Types[]={"T_OPERATOR","T_CONST","T_EXIT","T_UNDEFINED"};
			return Types[Type];
		}
		const char VarAsID() { return VarStack.top(); }
		const char CalcAsID()
		{
			for(char i=0;i<26;++i) { if(Consts[i]==CalcStack.top())return i+'A'; }
			return '`';
		}

		const bool CanBeASymbol(uint Val) { return CalcAsID()!='`'; }

		ErrorType Add()
		{
			uint B,A;
			PrepareStackToMath(A,B);
			R=A+B;
			if(R<A)return E_OVERFLOW;
			CalcStack.push(R);
			return E_NONE;
		}

		ErrorType Subtract()
		{
			uint B,A;
			PrepareStackToMath(A,B);
			R=B-A;
			if(R>B)R=0;//return E_NEGATIVE;
			CalcStack.push(R);
			return E_NONE;
		}

		ErrorType Multiply()
		{
			uint B,A;
			PrepareStackToMath(A,B);
			R=A*B;
			if(A&&R/A!=B)return E_OVERFLOW;
			return E_NONE;
		}

		ErrorType Divide()
		{
			uint B,A;
			PrepareStackToMath(A,B);
			if(!A)return E_ZERODIVISION;
			R=B/A;
			CalcStack.push(R);
			return E_NONE;
		}

		ErrorType Concat()
		{
			uint R,B,A;
			PrepareStackToMath(A,B);
			string Cat=to_string(B)+to_string(A);
			string MaxCat=to_string(numeric_limits<uint>::max());
			if(Cat.size()>MaxCat.size())return E_OVERFLOW;
			for(auto& i:Cat)
			{
				if(i>MaxCat[&i-&Cat[0]])return E_OVERFLOW;
			}
			R=stoi(Cat);
			//if(R<B)return E_OVERFLOW;
			CalcStack.push(R);
			return E_NONE;
		}

		//Ready to work with any stack
		ErrorType Top()
		{

			if(bIsUsingCalcStack)
			{
				if(CalcStack.empty())return E_EmptyStackAccessing;
				cout<<CalcStack.top();
			}
			else
			{
				if(VarStack.empty())return E_EmptyStackAccessing;
				cout<<Variables[VarStack.top()-'a'];
			}
			return E_NONE;
		}

		//Ready to work with any stack
		ErrorType Symbol()
		{

			if(bIsUsingCalcStack)
			{
				if(CalcStack.empty())return E_EmptyStackAccessing;
				if(CalcStack.top()>127)return E_NOTSYMBOL;
				cout<<(char)CalcStack.top();
			}
			else
			{
				if(VarStack.empty())return E_EmptyStackAccessing;
				if(VarStack.top()>127)return E_NOTSYMBOL;
				cout<<(char)VarStack.top()-'a';
			}
			return E_NONE;
		}

		//Ready to work with
		//ErrorType NewLine() { cout<<"\n";return E_NONE; }
		//ErrorType Whitespace() { cout<<" ";return E_NONE; }

		//Ready to work with any stack
		ErrorType Shave()
		{

			if(bIsUsingCalcStack)
			{
				if(CalcStack.empty())return E_EmptyStackShaving;
				CalcStack.pop();
			}
			else
			{
				if(VarStack.empty())return E_EmptyStackShaving;
				VarStack.pop();
			}
			return E_NONE;
		}

		//Ready to work with any stack
		ErrorType Exit()
		{
			cout<<endl;
			return E_EXIT;
		}

		//Ready to work
		ErrorType PushConst(char Token)
		{

			CalcStack.push(TokenAsNumber(Token));
			//cout<<CalcStack.top()<<"pushed";
			return E_NONE;
		}

		//Ready to work
		ErrorType PushVariable(char Token)
		{

			VarStack.push(Token);
			//cout<<Token<<" pushed!"<<endl;
			return E_NONE;
		}

		ErrorType Assign()
		{

			if(!AreStacksFilled())return E_EmptyStackAccessing;
			return bIsUsingCalcStack?AssignToVar():AssignToCalc();
		}

		//Ready to work
		ErrorType AssignToCalc()
		{

			//if(bIsUsingCalcStack)return E_WRONGSTACK;
			//cout<<VarStack.top()<<" assigned to "<<CalcStack.top()<<endl;
			if(CalcStack.empty())CalcStack.push(0u);
			CalcStack.top()=Variables[VarStack.top()-'a'];
			Variables[VarStack.top()-'a']=0;
			VarStack.pop();
			return E_NONE;
		}

		//Ready to work
		ErrorType AssignToVar()
		{

			//if(!bIsUsingCalcStack)return E_WRONGSTACK;
			//cout<<CalcStack.top()<<" assigned to "<<VarStack.top()<<endl;
			if(VarStack.empty())return E_EmptyStackAccessing;
			Variables[VarStack.top()-'a']=CalcStack.top();
			CalcStack.pop();
			return E_NONE;
		}

		//Ready to work with any stack
		ErrorType SwitchStack()
		{

			bIsUsingCalcStack=!bIsUsingCalcStack;
			//cout<<(bIsUsingCalcStack?"Calc stack in use":"Var stack in use")<<endl;
			return E_NONE;
		}

		//Ready to work with any stack
		[[deprecated("Feature has been removed")]]
		ErrorType ID()
		{

			if(bIsUsingCalcStack)
			{
				if(CalcStack.empty())return E_EmptyStackAccessing;
				if(!CanBeASymbol(CalcStack.top()))return E_NOTSYMBOL;
				cout<<CalcAsID();
			}
			else
			{
				if(VarStack.empty())return E_EmptyStackAccessing;
				cout<<VarAsID();
			}
			return E_NONE;
		}

		//LOGICAL START

		ErrorType Less()
		{

			uint A,B;
			PrepareStackToMath(A,B);
			R=A<B;
			CalcStack.push(R);
			return E_NONE;
		}

		ErrorType Equal()
		{

			uint A,B;
			PrepareStackToMath(A,B);
			R=A==B;
			CalcStack.push(R);
			return E_NONE;
		}

		ErrorType Greater()
		{

			uint A,B;
			PrepareStackToMath(A,B);
			R=A>B;
			CalcStack.push(R);
			return E_NONE;
		}

		ErrorType Nor()
		{
			uint A,B;
			PrepareStackToMath(A,B);
			R=NOR(A,B);
			CalcStack.push(R);
			return E_NONE;
		}

		//LOGICAL END

		ErrorType ClearStack()
		{

			if(bIsUsingCalcStack)CalcStack={};
			else VarStack={};
			return E_NONE;
		}

		ErrorType ReverseStack()
		{

			if(bIsUsingCalcStack)
			{
				vector<uint> Temp;
				while(!CalcStack.empty()) { Temp.push_back(CalcStack.top());CalcStack.pop(); }
				for(auto& i:Temp)CalcStack.push(i);
			}
			else
			{
				vector<char> Temp;
				while(!VarStack.empty()) { Temp.push_back(VarStack.top());VarStack.pop(); }
				for(auto& i:Temp)VarStack.push(i);
			}
			return E_NONE;
		}

		ErrorType Repeat(size_t& Counter)
		{
			if(bIsInLoop)return E_NESTEDLOOP;
			bIsInLoop=true;
			RepeatIndex=Counter;
			return E_NONE;
		}

		ErrorType Until(size_t& Counter)
		{
			if(!bIsInLoop)return E_NOTINLOOP;
			bool CalcIsNotValid=CalcStack.empty()||!CalcStack.top();
			bool VarIsNotValid=VarStack.empty()||!VarStack.top();
			if(bIsUsingCalcStack)
			{
				if(CalcIsNotValid)
				{
					bIsInLoop=false;
					return E_NONE;
				}
			}
			else
			{
				if(VarIsNotValid)
				{
					bIsInLoop=false;
					return E_NONE;
				}
			}
			Counter=RepeatIndex;
		}

		ErrorType Exec(char Token,size_t& Counter)
		{
			switch(GetType(Token))
			{
				case T_SWITCH:return SwitchStack();
				case T_REVERSE:return ReverseStack();
				case T_TOP:return Top();
				case T_SYM:return Symbol();
				case T_SHAVE:return Shave();
				case T_CLEAR:return ClearStack();
				case T_NEWLINE:return NewLine();
				case T_WHITESPACE:return Whitespace();
				case T_EXIT:return Exit();
				case T_CONST:return PushConst(Token);
				case T_VARIABLE:return PushVariable(Token);
				case T_ASSIGN:return Assign();
				//Cannot be overflowed
				case T_ADD:return Add();
				case T_SUB:return Subtract();
				case T_DIV:return Divide();
				//Can be overflowed!!!
				case T_MULT:return Multiply();
				case T_CAT:return Concat();
				//Logical
				case T_LESS:return Less();
				case T_EQUAL:return Equal();
				case T_GREATER:return Greater();
				case T_NOR:return Nor();
				//Loop
				case T_REPEAT:return Repeat(Counter);
				case T_UNTIL:return Until(Counter);
			}
			return E_NONE;
		}

		bool IsErrorFatal(ErrorType Error)
		{
			return NOR(Error==E_NONE,Error==E_EXIT);
		}

		void PrintError(ErrorType Error)
		{
			switch(Error)
			{
				case E_NOTSYMBOL:cout<<"The top of the stack cannot be represented as a character!"<<endl;return;
				case E_ZERODIVISION:cout<<"You can't divide by zero!"<<endl;return;
				case E_OVERFLOW:cout<<"Unsigned overflow!"<<endl;return;
				case E_EmptyStackAccessing:cout<<"Attempting to access the empty top of the stack!!"<<endl;return;
				case E_EmptyStackShaving:cout<<"Trying to shave off the empty top of the stack!!"<<endl;return;
				case E_NEGATIVE:cout<<"Trying to get negative value!"<<endl;return;
				case E_NESTEDLOOP:cout<<"Trying to do a nested loop!"<<endl;return;
				case E_NOTINLOOP:cout<<"Attempting to end an unstarted loop!"<<endl;return;
				case E_WRONGSTACK:cout<<"Wrong stack is using!"<<endl;return;
			}
		}

		void ParseSource(string Source)
		{
			if(Source.find('!')==string::npos)
			{
				cout<<"Program finish command ('!') not found!"<<endl;
				return;
			}
			ErrorType ExecResult;
			size_t SourceSize=Source.size();
			char c;
			for(size_t i=0;i<SourceSize;++i)
			{
				c=Source[i];
				ExecResult=Exec(c,i);
				if(IsErrorFatal(Exec(c,i)))
				{
					PrintError(ExecResult);
					return;
				}
			}
		}
	public:
		Interpreter(string Source)
		{
			Clear();
			ParseSource(Source);
		}
		void ParseAgain(string Source)
		{
			ParseSource(Source);
		}
		void Clear()
		{
			for(auto& i:Variables)i=0;
			CalcStack={};
			VarStack={};
			bIsUsingCalcStack=true;
			bIsInLoop=false;
			R=0;
		}
	};
}

void AsASCII(std::string Str)
{
	for(auto s:Str)std::cout<<(s-0)<<" ";
	std::cout<<std::endl;
}

[[deprecated("Doesn't work yet")]]
std::string FromASCII(std::string Str)
{
	std::string Res;
	return Res;
}

void RemindASCII(bool Extended=false)
{
	for(unsigned char i=0;i<(Extended?255:127);++i)printf("[%c=%d]\n",i,i);
}

int main(int argc,char** argv)
{
	//AsASCII("Hello, world!");
	//const std::string HelloWorld="72_$~J1_$~10_8_$~10_8_$~11_1_$~44_$~32_$~11_9_$~11_1_$~11_4_$~10_8_$~J$~33_$!";
	//const std::string HelloWorld1="G2+$C+1-$7+$$3+$G-3+$A-2-$JB+1-$8-$3+$6-$8-$F-7-$!";
	//#@#F1+$#?#,#~@#F1+$#?# - print "{var1}=8 {var2}=2"
	//RemindASCII();
	using namespace std;
	//GALL::Interpreter&& Gall=GALL::Interpreter("G2+$C+1-$7+$$3+$G-3+$A-2-$JB+1-$8-$3+$6-$8-$F-7-$!");
	//system("pause");
	/*
	cout<<"ab\tNOR\tNOT a\tNOT b\tAND\tOR\tXOR\tEQL"<<endl;
	for(int a=0;a<2;++a)
	{
		for(int b=0;b<2;++b)
		{
			cout<<format
			(
				"{:b}{:b}\t{:b}\t{:b}\t{:b}\t{:b}\t{:b}\t{:b}\t{:b}\n",
				a,b,NOR(a,b),NOT(a),NOT(b),AND(a,b),OR(a,b),XOR(a,b),EQL(a,b)
			);
		}
	}
	*/
	GALL::Interpreter&& Gall=GALL::Interpreter("12`?!");
	return 0;
}
