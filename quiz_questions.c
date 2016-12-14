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
    while((Char = fgetc(FileHandle)) != '|')
    {
        Result.Answer[CharIndex++] = Char;
    }
    
    // TODO: Make FileHandle a double pointer
    // TODO: Don't close the filehandle, move it up to the next question.
    // TODO: If there's no next question, then close FileHandle and open new FileHandle at random location
    // TODO: On serverside, if everyone says "skip", then the server just jumps to a new random place in the file!
    fclose(FileHandle);
    
    return Result;
}