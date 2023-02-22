#include "open62541.h"
#include "datasource.h"
#include "model.h"

/*
Add Variable (w/o update)
Add TimeVariable (data callback update)
Add NumLines (datasource update)
Add Method
*/

void
addVariable(UA_Server* server)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = 42;
    UA_Variant_setScalarCopy(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "the answer");
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING_ALLOC(1, "the.answer");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME_ALLOC(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
        parentReferenceNodeId, myIntegerName,
        UA_NODEID_NULL, attr, NULL, NULL);

    /* allocations on the heap need to be freed */
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myIntegerNodeId);
    UA_QualifiedName_clear(&myIntegerName);
}

static void
beforeReadTime(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeid, void* nodeContext,
    const UA_NumericRange* range, const UA_DataValue* data) {

    UA_DateTime now = UA_DateTime_now();
    UA_Variant value;
    UA_Variant_setScalar(&value, &now, &UA_TYPES[UA_TYPES_DATETIME]);

    UA_Server_writeValue(server, *nodeid, value);

}

void
addCurrentTimeVariable(UA_Server* server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Current time - value callback");
    attr.dataType = UA_TYPES[UA_TYPES_DATETIME].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    

    UA_NodeId currentNodeId = UA_NODEID_STRING_ALLOC(1, "current-time-value-callback");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME_ALLOC(1, "current-time-value-callback");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr, NULL, NULL);

    UA_ValueCallback callback;
    callback.onRead = beforeReadTime;
    callback.onWrite = NULL;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);

    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&currentNodeId);
    UA_QualifiedName_clear(&currentName);
}

static UA_StatusCode
readCurrentNumLines(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext,
    UA_Boolean sourceTimeStamp, const UA_NumericRange* range,
    UA_DataValue* dataValue) {
    UA_Int32 num = (UA_Int32)countLines("example.txt");
    UA_Variant_setScalarCopy(&dataValue->value, &num,
        &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

void
addCurrentNumLinesVariable(UA_Server* server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Current number of lines - data source");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_NodeId currentNodeId = UA_NODEID_STRING_ALLOC(1, "current-num-lines-datasource");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME_ALLOC(1, "current-num-lines-datasource");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource variableDataSource;
    variableDataSource.read = readCurrentNumLines;
    variableDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
        parentReferenceNodeId, currentName,
        variableTypeNodeId, attr,
        variableDataSource, NULL, NULL);

    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&currentNodeId);
    UA_QualifiedName_clear(&currentName);
}

static UA_StatusCode
writeInFileMethodCallback(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionHandle,
    const UA_NodeId* methodId, void* methodContext,
    const UA_NodeId* objectId, void* objectContext,
    size_t inputSize, const UA_Variant* input,
    size_t outputSize, UA_Variant* output) {

    UA_String* inputStr = (UA_String*)input->data;
    std::string text;
    
    if (inputStr->length > 0) {
        text = std::string((const char*)inputStr->data, inputStr->length);
    }

    writeInFile(text);

    std::string outputText = "'" + text + "' was written in file.";
    UA_String out = UA_STRING_ALLOC(outputText.c_str());
    UA_Variant_setScalarCopy(output, &out, &UA_TYPES[UA_TYPES_STRING]);

    UA_String_clear(&out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Write in File was called");
    
    return UA_STATUSCODE_GOOD;
}

void
addWriteInFileMethod(UA_Server* server) {
    UA_Argument inputArgument;
    UA_Argument_init(&inputArgument);
    inputArgument.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "A String");
    inputArgument.name = UA_STRING_ALLOC("Input");
    inputArgument.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument.valueRank = UA_VALUERANK_SCALAR;

    UA_Argument outputArgument;
    UA_Argument_init(&outputArgument);
    outputArgument.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "Answer");
    outputArgument.name = UA_STRING_ALLOC("Output");
    outputArgument.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    outputArgument.valueRank = UA_VALUERANK_SCALAR;

    UA_MethodAttributes writeAttr = UA_MethodAttributes_default;
    writeAttr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "Writes text in .txt file");
    writeAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Write in File");
    writeAttr.executable = true;
    writeAttr.userExecutable = true;

    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 62541);
    UA_QualifiedName currentName = UA_QUALIFIEDNAME_ALLOC(1, "write-in-file");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);

    UA_Server_addMethodNode(server, currentNodeId,
        parentNodeId,
        parentReferenceNodeId,
        currentName,
        writeAttr, &writeInFileMethodCallback,
        1, &inputArgument, 1, &outputArgument, NULL, NULL);

    UA_Argument_clear(&inputArgument);
    UA_Argument_clear(&outputArgument);
    UA_MethodAttributes_clear(&writeAttr);
    UA_QualifiedName_clear(&currentName);
}