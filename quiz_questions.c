
// TODO: Computes the minimum number of insertion + deletions + substitutions in order to transform A -> B
internal int
GetEditDistance(char *A, char *B)
{
    int Result = 0;
    
     int LengthA = (int)strlen(A);
    int LengthB = (int)strlen(B);

    if((LengthA > 0) && (LengthB > 0))
    {
        int TableWidth = (LengthA + 1);
        int TableHeight = (LengthB + 1);
        int TableSizeInBytes = TableWidth*TableHeight*sizeof(int);
        int *MemoTable = (int *)malloc(TableSizeInBytes);
        memset(MemoTable, 0, TableSizeInBytes);
        
        for(int Index = 0; Index < TableWidth; Index++)
        {
            *GetArrayPointer(MemoTable, TableWidth, 0, Index) = Index;
        }
        
        for(int Index = 0; Index < TableHeight; Index++)
        {
            *GetArrayPointer(MemoTable, TableWidth, Index, 0) = Index;
        }
        
        for(int Row = 1; Row < TableHeight; Row++)
        {
            for(int Col = 1; Col < TableWidth; Col++)
            {
                int SubstituteDistance = *GetArrayPointer(MemoTable, TableWidth, Row - 1, Col - 1);
                SubstituteDistance += (tolower(A[Col - 1]) != tolower(B[Row - 1]));
                
                int DeleteDistance = *GetArrayPointer(MemoTable, TableWidth, Row, Col - 1) + 1;
                int LeaveDistance = *GetArrayPointer(MemoTable, TableWidth, Row - 1, Col) + 1;
                
                int MinDistance = Min(SubstituteDistance, Min(DeleteDistance, LeaveDistance));
                *GetArrayPointer(MemoTable, TableWidth, Row, Col) = MinDistance;
            }
        }
        
        Result = *GetArrayPointer(MemoTable, TableWidth, LengthB, LengthA);
    }
    else if(LengthA == 0)
    {
        Result = LengthB;
    }
    else
    {
        Result = LengthA;
    }
    
    return Result;
}

internal r32
NormalizedDistance(char *Message, question *Question)
{
    int AnswerLength = (int)strlen(Question->Answer);
    int EditDistance = GetEditDistance(Message, Question->Answer);
    
    r32 Result = ((r32)Min(EditDistance, AnswerLength) / AnswerLength);
    Assert((Result >= 0.0f) && (Result <= 1.0f));
    
    return Result;
}

internal question
GenerateQuestion(FILE *FileHandle)
{
    question Result = {0};
    
    int CharIndex = 0;
    char Char = fgetc(FileHandle);
    Assert(Char == '|');
    
    while((Char = fgetc(FileHandle)) != '|')
    {
        Result.Category[CharIndex++] = Char;
    }
    
    CharIndex = 0;
    while((Char = fgetc(FileHandle)) != '|')
    {
        Result.Question[CharIndex++] = Char;
    }
    
    CharIndex = 0;
    while((Char = fgetc(FileHandle)) != '\n')
    {
        Result.Answer[CharIndex++] = Char;
    }
    
    if(fgetc(FileHandle) == '|')
    {
    ungetc('|', FileHandle);
    }
    else
    {
        InvalidCodePath;
    }
    
    printf("answer: %s\n", Result.Answer);
    
    return Result;
}

internal void
SkipQuestion(FILE *FileHandle)
{
     fseek(FileHandle, 0L, SEEK_END);
    int FileSize = ftell(FileHandle);
    int RandomByteOffset = rand() % FileSize;
    
    rewind(FileHandle);
    fseek(FileHandle, RandomByteOffset, 0);
    
    char Char = fgetc(FileHandle);
    while((Char != EOF) && (Char != '\n'))
    {
        Char = fgetc(FileHandle);
    }
    if(Char == EOF)
    {
        SkipQuestion(FileHandle);
    }
    else
    {
        Char = fgetc(FileHandle);
        if(Char != '|')
        {
            SkipQuestion(FileHandle);
        }
        else
        {
            ungetc('|', FileHandle);
        }
    }
}