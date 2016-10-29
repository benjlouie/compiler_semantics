class SymbolTable
{
    private:
        struct SymNode {
            //Name of the node
            string name;
            //The variables in this node and their values
            unordered_map<string,SymTableVariable> variables;
            //The functions in this node and their expressions
            unordered_map<string,SymTableMethod> methods;
            //Where we came from 
            SymNode *parent;
            //
            vector<SymNode *> children;
        }
    public:
        SymbolTable(void);
}

class SymTableVariable 
{
    //Type
    string type;
    //Value??
}

class SymTableMethod
{
    //Return type
    string returnType;
    //Functions within it?
    vector<string> argTypes;
}
