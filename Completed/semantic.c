#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "subc.h"

#define SIZE 50

/*
    semantic analysis가 끝난 뒤 global list free, stack 내부 검사하면서 free
    stack pop될 때 free
    stack push할 때 할당
  
    {변수명, 함수명}, {구조체, 공용체, 열거형} 같은 네임스페이스를 공유하는 집합
*/

/************* 전역 변수 **************/

TypeInfo *GlobalTypeHead;
FunctionInfo *GlobalFunctionHead;

// NULL이 아니면 Push 내부에서 매개변수를 심볼테이블에 복사
Variable *TempParameterList = NULL;
FunctionInfo *TempFunction = NULL;

int Top = -1;
int StackCount = 0;
Stack SymbolStack[SIZE];

/************* 함수 구현부 **************/
void Push()
{
    if(Top == (SIZE -1))
    {
        printf("Stack Full\n");
        return;
    }

    Top++;
    // 변수헤드의 다음 노드가 존재하지 않으면 할당
    if(!SymbolStack[Top].SymbolTable.VariableHead)
    {
        // Pop할 때 헤드에 연결된 리스트만 해제하고 헤드는 살려둔다.
        // 스택포인터가 가리키지 않는다면 논리적으로 없는 것
        // Pop할 때 헤드 해제했는데 다시 Push하면 동적할당을 다시 해줘야함
        // 메모리 유지 후 프로그램 종료 시 모든 동적할당 메모리 해제
        SymbolStack[Top].SymbolTable.VariableHead = (Variable*)calloc(1, sizeof(Variable));
        NULLCHECK(SymbolStack[Top].SymbolTable.VariableHead);
    }
    SymbolStack[Top].SymbolTable.VariableHead->Next = NULL;
    // 스택에 들어온 원소 개수 세고 종료 전에 스택을 순회하며 모든 동적할당 메모리 free
    StackCount = Top + 1;

    // 임시파라미터리스트가 존재하면 함수 scope open, 매개변수 == 지역변수
    if(TempParameterList)
    {
        CopyParameterList();
        TempParameterList = NULL;
    }
}

void Pop()
{
    // 스택에 존재하는 심볼테이블의 동적할당 메모리 해제
    // Top만 이동하면 논리적으로 제거된것과 동일
    // 해제하지 말고 멤버변수, FunctionList, TypeList만 해제하고 재사용
    // 글로벌 리스트는 제외
    if(Top == -1)
    {
        printf("Stack Empty\n");
        return;
    }
    
    // SymbolTableHead가 가리키는 리스트 해제
    // Head를 재사용하기 위해서 Head는 해제하지 않는다.
    DeleteSymbolTableVariable();
    // Top에 할당된 SymbolTableHead는 해제하지 않는다.
    // 추후 scope이 인식되면 재사용
    Top--;
}

void AllGlobalListFree()
{
    if(GlobalFunctionHead)
    {
        FunctionInfo *CurrentFunc = GlobalFunctionHead->Next;
        FunctionInfo *NextFunc;
        while(CurrentFunc)
        {
            NextFunc = CurrentFunc->Next;

            if(CurrentFunc->FunctionName)
                free(CurrentFunc->FunctionName);

                
            if(CurrentFunc->ParameterHead)
            {
                Variable *CurrentParam = CurrentFunc->ParameterHead->Next;
                Variable *NextParam;
                while(CurrentParam)
                {
                    NextParam = CurrentParam->Next;
                    if(CurrentParam->Name)
                        free(CurrentParam->Name);
                        
                    DeleteTypeInfoList(CurrentParam->TypeHead);
                    free(CurrentParam);
        
                    CurrentParam = NextParam;
                }
                free(CurrentFunc->ParameterHead);                    
            }
            
            if(CurrentFunc->ReturnType)
            {
                DeleteTypeInfoList(CurrentFunc->ReturnType);
                free(CurrentFunc->ReturnType);
            }
            free(CurrentFunc);

            CurrentFunc = NextFunc;
        }

        free(GlobalFunctionHead);
    }

    if(GlobalTypeHead)
    {
        TypeInfo* Current = GlobalTypeHead->Next;
        while(Current)
        {
            TypeInfo * Next = Current->Next;

            if(Current->StructName)
                free(Current->StructName);
            if(Current->StructFieldInfo)
                free(Current->StructFieldInfo);
    
            free(Current);
            Current = Next;
        }
    
        free(GlobalTypeHead);
        GlobalTypeHead = NULL;
    }
}

void GlobalTypeListInitialize()
{
    GlobalTypeHead = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(GlobalTypeHead);
    GlobalTypeHead->Next = NULL;
    
    // 함수리스트의 헤드도 같이 초기화
    GlobalFunctionHead = (FunctionInfo*)malloc(sizeof(FunctionInfo));
    NULLCHECK(GlobalFunctionHead);
    GlobalFunctionHead->Next = NULL;


    GlobalFunctionHead->ParameterHead = (Variable*)malloc(sizeof(Variable));
    NULLCHECK(GlobalFunctionHead->ParameterHead);
    GlobalFunctionHead->ReturnType = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(GlobalFunctionHead->ReturnType);

    GlobalFunctionHead->ParameterHead->Next =  NULL;
    GlobalFunctionHead->ReturnType->Next = NULL;
  
    for (int i = 0; i < 3; i++)
    {
        // 할당 및 0으로 초기화
        TypeInfo* New = (TypeInfo*)calloc(1, sizeof(TypeInfo));
        NULLCHECK(New);
        // 0부터 순서대로 INT, CHAR, VOID(NULL)
        New->Type = i;
        New->Next = GlobalTypeHead->Next;
        GlobalTypeHead->Next = New;
        
    }   
}

void AddTypeInfoListHead(TypeInfo* Head, TypeInfo *New)
{
    NULLCHECK(Head);
    NULLCHECK(New);
    // 헤드는 첫 번째로 기본 자료형을 참조하게 되어 있음, 만약 참조하지 않는다면 기본 자료형 삽입 순서가 잘못 된 것, 항상 type, pointer 먼저 삽입
    NULLCHECK(Head->Next);
    
    New->Next = Head->Next;
    Head->Next = New;     
}

int LookUpTypeGlobalList(TypeInfo *Item)
{
    for(TypeInfo *Current = GlobalTypeHead->Next; Current; Current = Current->Next)
    {
        if(Current == Item)
        {
            return 1;
        }
    }
    
    return 0;    
}

void DeleteTypeInfoList(TypeInfo* Head)
{
    NULLCHECK(Head);

    TypeInfo *Current = Head->Next;
    while(Current)
    {
        // 글로벌리스트는 제외
        if (LookUpTypeGlobalList(Current)) break;

        TypeInfo *Next = Current->Next;

        if(Current->StructName)
            free(Current->StructName);
        if(Current->StructFieldInfo)
            free(Current->StructFieldInfo);

        free(Current);
        Current = Next;
    }
    Head->Next = NULL;

}

void DeleteSymbolTableVariable()
{
    Variable *Current = SymbolStack[Top].SymbolTable.VariableHead->Next;
    Variable *Next;


    while(Current)
    {

        Next = Current->Next;

        if(Current->Name)
        {
            free(Current->Name);
        }

        DeleteTypeInfoList(Current->TypeHead);

        free(Current);

        Current = Next;
    }
   
    SymbolStack[Top].SymbolTable.VariableHead->Next = NULL;

}

TypeInfo *PointingTypeList(int TargetType)
{
    TypeInfo *Address = NULL;
    for(TypeInfo* Current = GlobalTypeHead->Next; Current; Current = Current->Next)
    {
        if(Current->Type == TargetType)
        {
            Address = Current;
            break;
        }
    }    

    NULLCHECK(Address);

    return Address;
}

TypeInfo* InsertVariable(char *Token01, char *Token02, char *ID)
{
    // Token02는 포인터 들어옴, 포인터는 NULL 허용
    NULLCHECK(Token01);

    // 배열은 InserVariable함수 두 번 호출 -> 인자로 ($1, $2, $3), ("[", "]", NULL)
    if(ID)
    {
        Variable *New = (Variable*)malloc(sizeof(Variable));
        NULLCHECK(New);
        New->Next = NULL;
    
        // 변수의 TypeInfo를 저장하기 위한 리스트헤드 초기화
        New->TypeHead = (TypeInfo*)malloc(sizeof(TypeInfo));
        NULLCHECK(New->TypeHead);
        New->TypeHead->Next = NULL;
        
        // 변수명 저장
        New->Name = (char*)malloc((strlen(ID) + 1) * sizeof(char));
        strcpy(New->Name, ID);
        New->Next = SymbolStack[Top].SymbolTable.VariableHead->Next;
        SymbolStack[Top].SymbolTable.VariableHead->Next = New;
    }
    TypeInfo *VariableType = InsertTypeInfo(Token01, Token02);

    return VariableType;
}

TypeInfo* InsertTypeInfo(char *Token01, char *Token02)
{
    // 함수 호출 시 type, pointer -> [, ] 순으로 호출 할 것
    // 배열 추가 하는 부분 분리 고려
    if(Token01 && Token02 && strcmp(Token01, "[") == 0 && strcmp(Token02, "]") == 0)
    {
        // array info 추가 코드 작성
        TypeInfo *Array = (TypeInfo*)malloc(sizeof(TypeInfo));
        Array->Type = ARRAY;
        AddTypeInfoListHead(SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead, Array);
        // type, * 먼저 호출하면 이미 메모리 할당되어 있음
        // 만약 []가 들어오면 이미 존재하므로 for문 아래의 코드 실행되면 안된다.
        return SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead->Next;
    }

    // SymbolTable에 저장된 변수의 TypeHead 검사
    NULLCHECK(SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead);
    SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead->Next = NULL;

    // GetDefaultType함수로 Token01의 기본자료형 검사, PointeingTypelist로 기본자료형 연결
    Types DefaultType = GetDefaultType(Token01);
    // 입력된 토큰이 기본자료형이 아니면 GetDefautType은 NONE을 반환
    if(DefaultType > NONE)
    {
        SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead->Next = PointingTypeList(DefaultType);
    }
    // 기본 자료형이 아니면 구조체
    // 기본 자료형아 아니라면 Token01은 구조체명이 들어온다
    else
    {
        SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead->Next = GetStructType(Token01);
    }
    
    // Token02 = * -> 존재하면 리스트에 추가
    if(Token02)
    {
        TypeInfo *Pointer = (TypeInfo*)calloc(1, sizeof(TypeInfo));
        NULLCHECK(Pointer);
        Pointer->Next = NULL;
        Pointer->Type = POINTER;
        AddTypeInfoListHead(SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead, Pointer);
    }

    // for(TypeInfo *t = SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead->Next; t; t = t->Next)
    // {
    //     printf(" %d ", t->Type);
    // }
    // printf("\n");
    // 코드 재사용을 위해서 심볼테이블의 타입헤드 다음의 노드 반환(실제 Type을 지정하는 노드)
    // 이 함수로 param_decl의 action 수행
    // 반환되는 노드 주소를 FunctionInfo의 ParameterHead에 연결
    // 매개변수노드의 주소는 심볼테이블의 타입헤드와 파라미터헤드 두 개의 헤드가 가리킴 - 이 부분 잘못됨
    // 심볼테이블 타입헤드와 함수의 파라미터 헤드가 따로 노드를 가져야함
    // Pop하면 심볼테이블의 지역변수(파라미터도 지역변수)를 제거하기 때문에 함수의 파라미터 소실
    // 반환된 정보로 새로운 리스트 생성    
    return SymbolStack[Top].SymbolTable.VariableHead->Next->TypeHead;
}

ExtendedTypeInfo *LookUpVariable(char *ID)
{
    NULLCHECK(ID);

    ExtendedTypeInfo *Info = NULL;

    for(Variable *Current = SymbolStack[Top].SymbolTable.VariableHead->Next; Current; Current = Current->Next)
    {
        // 헤드를 제외한 첫 노드 반환, return 검사할 때의 용이성을 위함
        if(Current->Name && strcmp(Current->Name, ID) == 0)
        {
            // 변수명이 존재하면 해당 변수의 TypeInfo 반환
            Info = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
            NULLCHECK(Info);
            Info->Type = Current->TypeHead->Next;
            Info->IsLvalue = 1;
            Info->flag = 0;
            break;
        }
    }

    if(Info && Info->Type)
    {
        for(TypeInfo *Current = Info->Type; Current; Current = Current->Next)
        {
            // 배열명은 RValue
            if(Current->Type == ARRAY)
            {
                Info->IsLvalue = 0;
            }
        }
    }
    
    return Info;
}

int AssignmentTypeCheck(ExtendedTypeInfo* TypeHead01, ExtendedTypeInfo* TypeHead02)
{   
    NULLCHECK(TypeHead01);
    NULLCHECK(TypeHead02);

    int flag = 1;
    TypeInfo *LHS = TypeHead01->Type;
    TypeInfo *RHS = TypeHead02->Type;
    // 우선 함수 반환형 부터 비교
    while(LHS && RHS)
    {
        /*
        1. 둘 다 존재할것(NULL이 아닐것)
        2. 두 개의 타입이 같을 것
        1, 2를 만족하면 다음 노드로 이동하고 continue
        1 or 2를 만족하지 못하면 flag를 0으로 만들고 break;
        */
        
        // 포인터면 다음거 검사, 이 함수 호출된 거면 RHS는 LValue임
        if(LHS->Type == POINTER)
        {
            if(TypeHead02->flag == 0)
            {
                flag = 0;
                break;
            }
            LHS = LHS->Next;
        }
        else if(RHS->Type == POINTER)
        {
            RHS = RHS->Next;
        }
        if(LHS->Type != RHS->Type)
        {
            flag = 0;
            break;         
        }
        
        LHS = LHS->Next;
        RHS = RHS->Next;        
    }
    TypeHead01->Type = NULL;
    TypeHead02->Type = NULL;
    free(TypeHead01);
    free(TypeHead02);

    return flag;
}

void DeclareFunction(char* Type, char* Pointer, char *ID)
{
    // 포인터는 NULL일 수 있으니까 검사 x
    NULLCHECK(Type);
    NULLCHECK(ID);

    // 새로운 함수 생성
    FunctionInfo *New = (FunctionInfo*)malloc(sizeof(FunctionInfo));
    NULLCHECK(New);
    New->Next = NULL;
    // 매개변수가 존재할 경우에 동적할당 - AddParameterListHead함수에서 동적할당 담당
    New->ParameterHead = NULL;

    // 변수명 저장
    New->FunctionName = (char*)malloc(sizeof(char) * (strlen(ID) + 1));
    NULLCHECK(New->FunctionName);
    strcpy(New->FunctionName, ID);    

    // 반환 타입 지정 - 기본 자료형 먼저 리스트에 연결
    New->ReturnType = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(New->ReturnType);
    New->ReturnType->Next = NULL;
    Types DefaultType = GetDefaultType(Type);    
    if(DefaultType > NONE)
    {
        New->ReturnType->Next = PointingTypeList(DefaultType);
    }

    // 매개변수 Pointer가 존재하면
    if(Pointer)
    {
        TypeInfo *p = (TypeInfo*)malloc(sizeof(TypeInfo));
        NULLCHECK(p);
        p->Next = NULL;
        p->Type = POINTER;
        AddTypeInfoListHead(New->ReturnType, p);
    }
    New->Next = GlobalFunctionHead->Next;
    GlobalFunctionHead->Next = New;
}

FunctionInfo *LookUpFunction(char *ID)
{
    NULLCHECK(ID);

    for(FunctionInfo *Current = GlobalFunctionHead->Next; Current; Current = Current->Next)
    {
        // 존재하면 true 반환, 오류 출력
        if(strcmp(Current->FunctionName, ID) == 0)
        {
            TempParameterList = Current->ParameterHead;
            TempFunction = Current;
            return Current;
        }
    }

    return NULL;
}

void AddFunctionInfoList(FunctionInfo *New)
{
    NULLCHECK(New);

    New->Next = GlobalFunctionHead->Next;
    GlobalFunctionHead->Next = New;
}

int CompareReturnType(ExtendedTypeInfo *TypeHead)
{
    NULLCHECK(TypeHead);
    
    // 항상 리스트의 앞에 추가 -> GlobalFunctionHead->Next는 항상 새로 생성된 함수
    // 함수 선언 시에는 항상 scope open, 헤드의 가장 앞에 있는 함수의 반환형 검사하면 될듯
    TypeInfo *FuncReturnType = GlobalFunctionHead->Next->ReturnType->Next;
    TypeInfo *Target = TypeHead->Type;
    int flag = 1;
    
    // 만약 void타입 함수가 존재하면 이 함수 알고리즘은 성립하지 않음, void타입에 대한 추가 처리 필요
    // void인 경우 return문 자체가 없거나 return; 만 존재해야함
    while(Target && FuncReturnType)
    {
        /*
        1. 둘 다 존재할것(NULL이 아닐것)
        2. 두 개의 타입이 같을 것
        1, 2를 만족하면 다음 노드로 이동하고 continue
        1 or 2를 만족하지 못하면 flag를 0으로 만들고 break;
        */
        if((Target && FuncReturnType) && (Target->Type == FuncReturnType->Type))
        {
            Target = Target->Next;
            FuncReturnType = FuncReturnType->Next;
            continue;            
        }
        
        flag = 0;
        break;
    }

    TypeHead->Type = NULL;
    free(TypeHead);

    return flag;
}

int GetDefaultType(char *ID)
{
    NULLCHECK(ID);

    for(int i = 0; i < 3; i++)
    {
        const char *t[] = {"int", "char", "NULL"};
        if(strcmp(t[i], ID) == 0)
        {
            return i;
        }
    }

    return NONE;
}

Variable *AddParameterListHead(Variable *Parameter)
{
    NULLCHECK(Parameter);
    
    GlobalFunctionHead->Next->ParameterHead = (Variable*)malloc(sizeof(Variable));
    NULLCHECK(GlobalFunctionHead->Next->ParameterHead);
    GlobalFunctionHead->Next->ParameterHead->Next = Parameter;

    return Parameter;
}

Variable *DeclareParameter(char *Token01, char *Token02, char *ID)
{
    NULLCHECK(Token01);

    // 매개변수 생성
    Variable *New = (Variable*)malloc(sizeof(Variable));        
    NULLCHECK(New);
    New->Next = NULL;

    // 변수명 저장
    New->Name = (char*)malloc(sizeof(char) * (strlen(ID) + 1));
    NULLCHECK(New->Name);
    strcpy(New->Name, ID);

    // 매개변수의 타입 연결
    New->TypeHead = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(New->TypeHead);
    New->TypeHead->Next = NULL;
    Types DefaultType = GetDefaultType(Token01);
    if(DefaultType > NONE)
    {
        New->TypeHead->Next = PointingTypeList(DefaultType);
    }

    // 포인터가 존재하면
    if(Token02)
    {
        TypeInfo *Pointer = (TypeInfo*)malloc(sizeof(TypeInfo));
        NULLCHECK(Pointer);
        Pointer->Next = NULL;
        Pointer->Type = POINTER;
        AddTypeInfoListHead(New->TypeHead, Pointer);
    }

    return New;
}

Variable *AddArrayType(Variable *NewVariable)
{
    NULLCHECK(NewVariable);
    Variable *New = NewVariable;

    TypeInfo *Array = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(Array);
    Array->Next = NULL;
    Array->Type = ARRAY;

    AddTypeInfoListHead(New->TypeHead, Array);

    return New;
}

Variable *CreateParameterList(Variable *Param01, Variable *Param02)
{
    NULLCHECK(Param01);
    NULLCHECK(Param02);

    // 먼저 생성된 매개변수가 뒤로 가도록 리스트 구성
    Param02->Next = Param01;

    return Param02;
}

void CopyParameterList()
{
    Variable *Head = NULL;
    Variable *Tail = NULL;

    for(Variable *Current = TempParameterList; Current; Current = Current->Next)
    {
        Variable *New = CopyParameterNode(Current);
    
        // 리스트 순서 유지
        if(Head)
        {
            Tail->Next = New;
            Tail = New;
        }
        else
        {
            Head = New;
            Tail = New;            
        }
    }

    SymbolStack[Top].SymbolTable.VariableHead->Next = Head;
}

Variable *CopyParameterNode(Variable *Original)
{
    NULLCHECK(Original);

    Variable *New = (Variable*)malloc(sizeof(Variable));
    NULLCHECK(New);
    New->Next = NULL;

    New->Name = (char*)malloc(sizeof(char) * (strlen(Original->Name) + 1));
    strcpy(New->Name, Original->Name);

    // 복사한 변수의 타입정보리스트 복사
    New->TypeHead = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(New->TypeHead);
    New->TypeHead->Next = CopyTypeInfoList(Original->TypeHead->Next);

    return New;
}

TypeInfo *CopyTypeInfoList(TypeInfo *Original)
{
    NULLCHECK(Original);

    TypeInfo *Head = NULL;
    TypeInfo *Tail = NULL;
    
    /* 구조체에 대한 부분은 생략 */
    for(TypeInfo *Current = Original; Current; Current = Current->Next)
    {
        TypeInfo *New = NULL;

        // 기본자료형이면 전역리스트의 노드 가리킨다
        // 0 : INT, 1 : CHAR
        if(Current->Type < 2)
        {
            New = PointingTypeList(Current->Type);

            if(Tail)
            {
                Tail->Next = New;
            }
            else
            {
                Head = New;
            }

            // 기본자료형은 리스트의 마지막이니 break
            break;
        }
        // 기본자료형 외에는 메모리 할당하고 복사
        else
        {
            New = (TypeInfo*)malloc(sizeof(TypeInfo));
            NULLCHECK(New);
            New->Next = NULL;
            New->Type = Current->Type;
        }

        // 리스트 순서 유지
        if(Head)
        {
            Tail->Next = New;
            Tail = New;
        }
        else
        {
            Head = New;
            Tail = New;            
        }
    }

    return Head;
}

ExtendedTypeInfo *BinaryOperandTypeCheck(ExtendedTypeInfo *TypeHead01, ExtendedTypeInfo *TypeHead02)
{
    NULLCHECK(TypeHead01);
    NULLCHECK(TypeHead02);

    if(TypeHead01->Type->Type == INT && TypeHead02->Type->Type == INT)
    {
        // TypeHead01->Type = NULL;
        // TypeHead02->Type = NULL;
        // free(TypeHead01);
        // free(TypeHead02);
        ExtendedTypeInfo *Info = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
        Info->Type = PointingTypeList(INT);
        Info->IsLvalue = 1;
        Info->flag = 0;
        return Info;
    }
    
    // if(TypeHead01->IsLvalue == 0 || TypeHead02->IsLvalue == 0)
    // {
    //     TypeHead01->Type = NULL;
    //     TypeHead02->Type = NULL;
    //     return NULL;
    // }
    // int flag = 1;
    // TypeInfo *LHS = TypeHead01->Type;
    // TypeInfo *RHS = TypeHead02->Type;
    // while(LHS && RHS)
    // {
    //     /*
    //     1. 둘 다 존재할것(NULL이 아닐것)
    //     2. 두 개의 타입이 같을 것
    //     1, 2를 만족하면 다음 노드로 이동하고 continue
    //     */
    //     if((LHS && RHS) && (LHS->Type == RHS->Type))
    //     {
    //         LHS = LHS->Next;
    //         RHS = RHS->Next;
    //         continue;            
    //     }
        
    //     flag = 0;
    //     break;
    // }
    // 타입이 같다면 둘 중 하나의 타입 반환, 아니라면 NULL반환
    // ExtendedTypeInfo *OperationType = flag ? TypeHead01 : NULL;

    // TypeHead01->Type = NULL;
    // TypeHead02->Type = NULL;
    // free(TypeHead01);
    // free(TypeHead02);

    return NULL;
}

ExtendedTypeInfo **CreateArgumentList(ExtendedTypeInfo** ArgList, ExtendedTypeInfo* New)
{
    NULLCHECK(ArgList);
    NULLCHECK(New);

    for(int i = 0; i < 10; i++)
    {
        if(!ArgList[i])
        {
            ArgList[i] = New;
            break;
        }       
    }

    return ArgList;
}

ExtendedTypeInfo *ArgumentTypeCheck(ExtendedTypeInfo **ArgumentList)
{
    NULLCHECK(ArgumentList);
    NULLCHECK(TempFunction);

    FunctionInfo *Func = TempFunction;
    TempFunction = NULL;

    if(!Func->ParameterHead)
    {
        DeleteArgumentList(ArgumentList);
        return NULL;
    }        

    int i = 9;
    for(; i >= 0; i--)
    {
        if(ArgumentList[i]) break;
    }

    Variable *Parameter = Func->ParameterHead->Next;
    while(Parameter && i > -1)
    {
        TypeInfo *ArgumentType = ArgumentList[i]->Type;
        TypeInfo *ParameterType = Parameter->TypeHead->Next;
        if((ArgumentList[i]->IsLvalue = 1) && (ParameterType->Type == POINTER))
        {
            ParameterType = ParameterType->Next;
            if(ArgumentType->Type != ParameterType->Type)
            {
                DeleteArgumentList(ArgumentList);
                return NULL;
            }
        }

        while(ArgumentType && ParameterType)
        {
            if(ArgumentType->Type != ParameterType->Type)
            {
                DeleteArgumentList(ArgumentList);
                return NULL;
            }
            ArgumentType = ArgumentType->Next;
            ParameterType = ParameterType->Next;
        }
        i--;
        Parameter = Parameter->Next;
    }

    if((i == -1) && !Parameter)
    {
        DeleteArgumentList(ArgumentList);
        ExtendedTypeInfo *ReturnType = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
        ReturnType->Type = Func->ReturnType->Next;
        ReturnType->flag = 0;
        if(ReturnType->Type->Type == ARRAY)
        {
            ReturnType->IsLvalue = 0;
        }
        return ReturnType;
    }

    DeleteArgumentList(ArgumentList);
    return NULL;
}

ExtendedTypeInfo **CopyArgument(ExtendedTypeInfo *Argument)
{
    NULLCHECK(Argument);

    ExtendedTypeInfo **Args = (ExtendedTypeInfo**)calloc(10, sizeof(ExtendedTypeInfo*));

    Args[0] = Argument;

    return Args;
}

ExtendedTypeInfo *GetDefaultTypeConst(int num)
{
    ExtendedTypeInfo *Info = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
    NULLCHECK(Info);
    Info->Type = PointingTypeList(num);
    Info->IsLvalue = 0;
    Info->flag = 0;

    return Info;
}

ExtendedTypeInfo *GetFunctionReturnType()
{
    NULLCHECK(TempFunction);
    //printf("getd");
    FunctionInfo *Function = TempFunction;
    TempFunction = NULL;
    ExtendedTypeInfo *ReturnInfo = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
    ReturnInfo->Type = Function->ReturnType->Next;
    ReturnInfo->flag = 0;
    if(ReturnInfo->Type->Type == ARRAY)
    {
        // 배열명은 RVlaue
        ReturnInfo->IsLvalue = 0;
    }

    return ReturnInfo;
}

void DeleteArgumentList(ExtendedTypeInfo **List)
{
    NULLCHECK(List);
    
    for(int i = 0; i < 10; i++)
    {
        List[i] = NULL;
    }

    free(List);
}

ExtendedTypeInfo *LValueCheck(ExtendedTypeInfo *Target)
{
    NULLCHECK(Target);

    if(Target->IsLvalue == 1)
    {
        Target->flag = 1;
        return Target;
    }

    return NULL;
}

ExtendedTypeInfo *ArrayElementCheck(ExtendedTypeInfo *Array, ExtendedTypeInfo *Index)
{
    NULLCHECK(Array);
    NULLCHECK(Index);

    // 배일명인 경우 0
    if(Array->IsLvalue == 1)
    {
        Array->Type = NULL;
        Index->Type = NULL;
        free(Array);
        free(Index);
        return NULL;
    }

    TypeInfo *IndexType = Index->Type;
    TypeInfo *ArrayType = Array->Type;

    // 인덱스가 int형이 아니면 false
    if(IndexType->Type != INT)
    {
        Array->Type = NULL;
        Index->Type = NULL;
        free(Array);
        free(Index);
        return NULL;
    } 

    for(TypeInfo *Current = ArrayType; Current; Current = Current->Next)
    {
        if(Current->Type < 2)
        {
            ExtendedTypeInfo *Result = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
            Result->Type = Current;
            Result->IsLvalue = 1;
            Result->flag = 0;
            return Result;            
        }
    }

    Array->Type = NULL;
    Index->Type = NULL;
    free(Array);
    free(Index);

    return NULL;
}

ExtendedTypeInfo *UnaryOperationCheck(ExtendedTypeInfo *Target)
{
    NULLCHECK(Target);

    if(Target->Type->Type < VOID)
    {
        return Target;
    }

    Target->Type = NULL;
    free(Target);

    return NULL;
}

int RelationOperationCheck(ExtendedTypeInfo *LHS, ExtendedTypeInfo *RHS)
{
    NULLCHECK(LHS);
    NULLCHECK(RHS);

    if(LHS->IsLvalue == 0 || RHS->IsLvalue == 0)
    {
        LHS->Type = NULL;
        RHS->Type = NULL;
        free(LHS);
        free(RHS);

        return 0;
    }

    if((LHS->Type->Type < 2) && (RHS->Type->Type < 2))
    {
        if(RHS->Type->Type == RHS->Type->Type)
        {
            LHS->Type = NULL;
            RHS->Type = NULL;
            free(LHS);
            free(RHS);
            return 1;
        }
    }

    LHS->Type = NULL;
    RHS->Type = NULL;
    free(LHS);
    free(RHS);

    return 0;
}

ExtendedTypeInfo *UnarySignCheck(ExtendedTypeInfo *Target)
{
    NULLCHECK(Target);

    if(Target->Type->Type == INT)
    {
        return Target;
    }

    Target->Type = NULL;
    free(Target);

    return NULL;
}

void DeclareStruct(char *ID)
{
    NULLCHECK(ID);

    TypeInfo *NewStruct = (TypeInfo*)malloc(sizeof(TypeInfo));
    NULLCHECK(NewStruct);
    NewStruct->Next = NULL;
    
    // 타입 지정
    NewStruct->Type = Struct;
    // 구조체 이름 지정
    NewStruct->StructName = (char*)malloc(sizeof(char) * (strlen(ID) + 1));
    NULLCHECK(NewStruct->StructName);
    strcpy(NewStruct->StructName, ID);    

    // 멤버변수 리스트 연결
    NewStruct->StructFieldInfo = (Variable*)malloc(sizeof(Variable));
    NULLCHECK(NewStruct->StructFieldInfo);
    NewStruct->StructFieldInfo->Next = NULL;

    // scope이 열리면서 멤버 변수 생성, 심볼테이블에 연결
    // 심볼테이블에 연결된 멤버 변수 리스트를 구조체정보에 연결, Pop되면서 해제되지 않도록 심볼테이블과 연결을 끊음
    NewStruct->StructFieldInfo->Next = SymbolStack[Top].SymbolTable.VariableHead->Next;
    SymbolStack[Top].SymbolTable.VariableHead->Next = NULL;

    for(TypeInfo *Current = GlobalTypeHead->Next; Current; Current = Current->Next)
    {
        // Head -> NULL -> CHAR -> INT -> STRUCT
        if(Current->Next == NULL)
        {
            Current->Next = NewStruct;
            break;
        }
    }
}

char *LookUpStruct(char *ID)
{
    NULLCHECK(ID);

    for(TypeInfo *Current = GlobalTypeHead->Next; Current; Current = Current->Next)
    {
        if(Current->StructName && strcmp(Current->StructName, ID) == 0)
        {
            return ID;
        }
    }

    return NULL;
}

TypeInfo *GetStructType(char *ID)
{
    NULLCHECK(ID);

    for(TypeInfo *Current = GlobalTypeHead->Next; Current; Current = Current->Next)
    {
        if(Current->StructName && strcmp(Current->StructName, ID) == 0)
        {
            return Current;
        }
    }

    return NULL;
}

ExtendedTypeInfo *LookUpStructMemberVar(ExtendedTypeInfo *Var, char *MemberName)
{
    NULLCHECK(Var);
    NULLCHECK(MemberName);

    Variable *StructInfo = Var->Type->StructFieldInfo;

    for(Variable *Member = StructInfo->Next; Member; Member = Member->Next)
    {
        if(Member->Name && strcmp(Member->Name, MemberName) == 0)
        {
            Var->Type = NULL;
            free(Var);
            ExtendedTypeInfo *Info = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
            Info->Type = Member->TypeHead->Next;
            Info->IsLvalue = 1; 
            Info->flag = 0;           
            return Info;
        }
    }

    return NULL;
}

ExtendedTypeInfo *StructOperationCheck(ExtendedTypeInfo *RHS, char *LHS)
{
    NULLCHECK(RHS);
    NULLCHECK(LHS);
    
    // RHS가 Pointer 타입인지 검사
    TypeInfo *RHSType = RHS->Type;
    for(; RHSType; RHSType = RHSType->Next)
    {
        // 첫 타입이 포인터가 아니먄 잘못된거임
        // array -> pointer -> defaulttype인데 array pointer는 채점 요소X
        if(RHSType->Type == POINTER)
        {
            RHSType = RHSType->Next;
            break;      
        }
        RHS->Type = NULL;
        free(RHS);
        return NULL;
    }

    // RHSType(struct)의 멤버 변수 탐색
    for(Variable *Member = RHSType->StructFieldInfo->Next; Member; Member = Member->Next)
    {
        if(Member->Name && strcmp(Member->Name, LHS) == 0)
        {
            RHS->Type = NULL;
            free(RHS);
            ExtendedTypeInfo *New = (ExtendedTypeInfo*)malloc(sizeof(ExtendedTypeInfo));
            New->Type = Member->TypeHead->Next;
            New->IsLvalue = 1;
            New->flag = 0;
            return New;
        }
    }
    

    RHS->Type = NULL;
    free(RHS);

    return NULL;    
}