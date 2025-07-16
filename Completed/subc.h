/*
 * File Name    : subc.h
 * Description  : A header file for the subc program.
 */

#ifndef __SUBC_H__
#define __SUBC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NULLCHECK(ptr)  do{                                 \
    if((ptr) == NULL){                                      \
        printf("%s : Memory Allocation Fail\n", #ptr);      \
        exit(1);}                                           \
    } while(0)

#define AddListHead(Type, Item) do{             \
    (Item)->Next = (GlobalType##Head)->Next;    \
    (GlobalType##Head)->Next = (Item);          \
    } while(0)

/********       구조체      ********/
// VOID(= NULL), Struct는 .tab.h의 STRUCT와 중복 회피
typedef enum Type{ NONE = -1, INT, CHAR, VOID, POINTER, ARRAY, Struct} Types;

typedef struct TypeInfo {
    Types Type;
    // Struct인 경우 Struct Name 저장
    char* StructName;
    // 구조체 field 정보
    struct Variable* StructFieldInfo;
    // 연결리스트
    struct TypeInfo* Next;
} TypeInfo;

typedef struct ExtendedTypeInfo {
    TypeInfo *Type;
    int IsLvalue;
    int flag;
} ExtendedTypeInfo;

typedef struct Variable {
    char* Name;
    struct TypeInfo* TypeHead;
    struct Variable* Next;
} Variable;

typedef struct FunctionInfo {
    char* FunctionName;
    Variable *ParameterHead;
    TypeInfo *ReturnType;
    struct FunctionInfo *Next;
} FunctionInfo;

typedef struct SymbolTable{
    struct Variable *VariableHead;
} SymbolTable;

typedef struct Stack{
    SymbolTable SymbolTable;
} Stack;

/********      전역변수     ********/
extern TypeInfo *GlobalTypeHead;
extern FunctionInfo *GlobalFunctionHead;
extern Stack SymbolStack[];
extern Variable *TempParameterList;
extern FunctionInfo *TempFunction;

/********     함수 선언부    ********/
void InitializeGlobalScopeStack();
void Push();
void Pop();
void AllGlobalListFree();
void GlobalTypeListInitialize();
void AddTypeInfoListHead(TypeInfo* Head, TypeInfo *New);
int LookUpTypeGlobalList(TypeInfo *Item);
void DeleteTypeInfoList(TypeInfo* Head);
void DeleteSymbolTableVariable();
TypeInfo *PointingTypeList(int TargetType);


TypeInfo* InsertVariable(char *Token01, char *Token02, char *ID);
TypeInfo* InsertTypeInfo(char *Token01, char *Token02);

ExtendedTypeInfo *LookUpVariable(char *ID);
int AssignmentTypeCheck(ExtendedTypeInfo* TypeHead01, ExtendedTypeInfo* TypeHead02);
void DeclareFunction(char* Type, char* Pointer, char *ID);
FunctionInfo *LookUpFunction(char *ID);
void AddFunctionInfoList(FunctionInfo *New);
int CompareReturnType(ExtendedTypeInfo *TypeHead);
int GetDefaultType(char *ID);
Variable *AddParameterListHead(Variable *Parameter);
Variable *DeclareParameter(char *Token01, char *Token02, char *ID);
Variable *AddArrayType(Variable *NewVariable);
Variable *CreateParameterList(Variable *Param01, Variable *Param02);
void CopyParameterList();
Variable *CopyParameterNode(Variable *Original);
TypeInfo *CopyTypeInfoList(TypeInfo *Original);
ExtendedTypeInfo *BinaryOperandTypeCheck(ExtendedTypeInfo *TypeHead01, ExtendedTypeInfo *TypeHead02);
ExtendedTypeInfo **CreateArgumentList(ExtendedTypeInfo** ArgList, ExtendedTypeInfo* New);
ExtendedTypeInfo *ArgumentTypeCheck(ExtendedTypeInfo **ArgumentList);
ExtendedTypeInfo **CopyArgument(ExtendedTypeInfo *Argument);
ExtendedTypeInfo *GetDefaultTypeConst(int num);
ExtendedTypeInfo *GetFunctionReturnType();
void DeleteArgumentList(ExtendedTypeInfo **List);
ExtendedTypeInfo *LValueCheck(ExtendedTypeInfo *Target);
ExtendedTypeInfo *ArrayElementCheck(ExtendedTypeInfo *Array, ExtendedTypeInfo *Index);
ExtendedTypeInfo *UnaryOperationCheck(ExtendedTypeInfo *Target);
int RelationOperationCheck(ExtendedTypeInfo *LHS, ExtendedTypeInfo *RHS);
ExtendedTypeInfo *UnarySignCheck(ExtendedTypeInfo *Target);
void DeclareStruct(char *ID);
char *LookUpStruct(char *ID);
TypeInfo *GetStructType(char *ID);
ExtendedTypeInfo *LookUpStructMemberVar(ExtendedTypeInfo *RHS, char *LHS);
ExtendedTypeInfo *StructOperationCheck(ExtendedTypeInfo *RHS, char *LHS);

#endif