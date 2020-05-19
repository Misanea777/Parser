#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cassert>



using ChildNodes = std::vector<std::unique_ptr<ASTNode>>;

// General base class - AST node
class ASTNode
{
    public:
        enum class NodeType
        {
            //Misce
            ERROR,
            // Decl
            VAR_DECL, FUN_DECL, VAR_PARAM_DECL, PROG_DECL,
            //Exp
            STR_EXP, NUM_EXP, IDENTIFIER_EXP, CALL_EXP, BINARY_EXP,
            //Statm
            COMPEX_STATM, RETURN_STATM
        };

    public:
        explicit ASTNode(NodeType type = NodeType::ERROR) : nType{type}, nName{} {}
        virtual ~ASTNode() = default;
    
    public:
        NodeType getType() const{return nType;}
        const std::string& getName() const{return nName;}
        const ChildNodes& getChildNodes() const{return nChildren;}

    protected:
        void addChildNode(std::unique_ptr<ASTNode>&& node) {nChildren.emplace_back(std::move(node));}

    protected:
        NodeType nType;
        std::string nName;
        ChildNodes nChildren;
};

//----------------------------------------------------------------------------------------------------------
// Types of possible Nodes

class Decl : public ASTNode
{
    public:
        explicit Decl(NodeType type) : ASTNode{type} {}
        explicit Decl() : ASTNode{ NodeType::ERROR } {}
        virtual ~Decl() = default;
};

class Exp : public ASTNode
{
    public:
        explicit Exp(NodeType type) : ASTNode{type} {}
        virtual ~Exp() = default;
        virtual bool isLiteral() const{return false;}
};

class Statm : public ASTNode
{
    public:
        explicit Statm(NodeType type) : ASTNode{type} {}
        virtual ~Statm() = default;
};

//----------------------------------------------------------------------------------------------------------
// Concrete classes

// Declaration Classes

class ProgramDecl : public Decl
{
    public:
        ProgramDecl() : Decl{NodeType::PROG_DECL} {}
        virtual ~ProgramDecl() = default;
    public:
        void addProgramDecl(std::unique_ptr<Decl> stm) {addChildNode(std::move(stm));}
        ChildNodes& getProgramDecl() {return nChildren;}
};

class VarDecl : public Decl
{
    public:
        VarDecl(const std::string& varName, int varValue, const std::string& varType) : Decl{NodeType::VAR_DECL} 
        {
            name = varName;
            value = varValue;
            type = varType;
        }
        virtual ~VarDecl() = default;
    
    public:
        bool addInitialization(std::unique_ptr<Exp> value)
        {
            if(value == nullptr) return false;
            else
            {
                addChildNode(std::move(value));
                return true;
            }
        }

        const std::string& getVarName() const{return name;}
        int getVarValue() {return value;}
        const std::string& getVarType() const{return type;}

    private:
        std::string name;
        int value; 
        std::string type;
};

class VarParamDecl : public Decl
{
    public:
        VarParamDecl() : Decl{NodeType::VAR_PARAM_DECL} {}
        virtual ~VarParamDecl() = default;
    
    public:
        void addParam(std::unique_ptr<VarDecl> param) {addChildNode(std::move(param));}
        const ChildNodes& getParam() const{return nChildren;}
};

class FunDecl : public Decl
{
    public:
        FunDecl() : Decl{NodeType::ERROR}, returnType{"ERROR"} {}
        FunDecl(const std::string& funName, const std::string& type, std::unique_ptr<VarParamDecl>&& parm, std::unique_ptr<ComplexStatm>&& body) : Decl{NodeType::FUN_DECL}, returnType{type}
        {
            nName = funName;
            addChildNode(std::move(parm));
            addChildNode(std::move(body));
        }
        virtual ~FunDecl() = default;

    public:
        const std::string& getFunName() const{return nName;}
        const std::string& getFunType() const{return returnType;}
        //need fun for getBody
        //need fun for getfunParam

    private:
        std::string returnType;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
// Expresions Classes

class BinOpExp : public Exp
{
    public:
        BinOpExp(char operation , std::unique_ptr<Exp> lhs, std::unique_ptr<Exp> rhs) : Exp{NodeType::BINARY_EXP} 
        {
            op = operation;
            if(lhs == nullptr || rhs == nullptr) abort();

            addChildNode(std::move(lhs));
            addChildNode(std::move(rhs));
        }
        virtual ~BinOpExp() = default;
    
    public:
        char getBinOp() {return op;}
        //need fun for get lhs and rhs
    
    private:
        char op;
};

class CallExp : public Exp
{
    public:
        CallExp(const std::string& funName, std::vector<std::unique_ptr<Exp>>&& args) : Exp{NodeType::CALL_EXP}
        {
            nName = funName;
            nChildren.insert(nChildren.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));
        }
        virtual ~CallExp() = default;

    public:
        const std::string& getCalledFunName() const{return nName;}
        const ChildNodes& getArgs() const{return nChildren;} 
};

class IdentifierExp : public Exp
{
    public: 
        IdentifierExp(const std::string& value) : Exp{NodeType::IDENTIFIER_EXP}
        {
            nName = value;
        }
        virtual ~IdentifierExp() = default;

    public:
        void setType(const std::string& associatedType) {type = associatedType;}
        const std::string& getType() const{return type;}
    
    private:
        std::string type;
};

class NumExp : public Exp
{
    public:
        NumExp(int numValue) : Exp{NodeType::NUM_EXP}, value{numValue} 
        {
            nName = std::to_string(value);
        }
        virtual ~NumExp() = default;
        virtual bool isLiteral() {return true;}
    
    public:
        const int getValue() const{return value;}
    
    private:
        int value;
};

class StrExp : public Exp
{
    public: 
        StrExp(const std::string& value) : Exp{NodeType::STR_EXP}
        {
            nName = value;
        }
        virtual ~StrExp() = default;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
// Statements Classes

class ComplexStatm : public Statm
{
    public:
        ComplexStatm() : Statm{NodeType::COMPEX_STATM} {}
        virtual ~ComplexStatm() = default;
    
    public:
        void addStatm(std::unique_ptr<ASTNode>&& statm) {addChildNode(std::move(statm));}
        const ChildNodes& getStatm() const{return nChildren;}
};

class ReturnStatm : public Statm
{
    public:
        ReturnStatm() : Statm{NodeType::RETURN_STATM} {}
        virtual ~ReturnStatm() = default;
    
    public:
        bool addReturnVal(std::unique_ptr<Exp>&& value) 
        {
            if(value == nullptr) return false;
            else
            {
                addChildNode(std::move(value));
                return true;
            }
        }

};



std::unique_ptr<ASTNode> Parser::ParseProgramDecl()
{
    auto programNode = std::make_unique<ProgramDecl>();
    mCurrentToken = mLexer.GetNextToken();
    
    std::unique_ptr<Decl> node = std::make_unique<Decl>();
    while (mCurrentToken != Lexer::Token::TOK_EOF)
    {
        switch (mCurrentToken)
        {
        case Lexer::Token::VAR:
            node.reset(ParseVarDecl().release());
            break;
        case Lexer::Token::FUNCTION:
             node.reset(ParseFunctionDecl().release());
            break;
        default:
            break;
        }
        
        programNode->AddProgramDecl(std::move(node));

        while ((mCurrentToken != Lexer::Token::VAR) 
            && (mCurrentToken != Lexer::Token::FUNCTION) 
            && (mCurrentToken != Lexer::Token::TOK_EOF))
            mCurrentToken = mLexer.GetNextToken();
    }

    return std::move(programNode);
}


std::unique_ptr<VarDecl> Parser::ParseVarDecl()
{
    std::unique_ptr<VarDecl> node = std::make_unique<VarDecl>();


    Common::Type vType = mLexer.GetCurrentType();


    if (vType == Common::Type::VOID)
    {
        cout<<"ERROR void is not a type for var"
        return std::move(node);
    }

      
    if ((mCurrentToken = mLexer.GetNextToken()) != Lexer::Token::IDENTIFIER)
    {
        return std::move(node);
    }
    const std::string varName = mLexer.GetCurrentStr();


    if (mCurrentToken == Lexer::Token::ASSIGN)
    {
        mCurrentToken = mLexer.GetNextToken();
        if (!vDecl->AddInitialization(ParseExpr()))
        {
            delete vDecl;
            return std::move(node);
        }
    }

    if (mCurrentToken == Lexer::Token::SEMI_COLON)
    {
        node.reset(vDecl);
        return std::move(node);
    }
    else
    {
        cout<<"ERROR! expected ;"
        delete vDecl;
        return std::move(node);
    }
}


std::unique_ptr<Expr> Parser::ParseBinaryOpExpr(Lexer::Token op, std::unique_ptr<Expr>&& lhs)
{
    

    mCurrentToken = mLexer.GetNextToken();

    std::unique_ptr<Expr> rhs = ParseExpr();
    if (rhs == nullptr)
    {
        
        return nullptr;
    }
    else
    {
        return std::make_unique<BinaryOpExpr>(TokenToOpcode(op), std::move(lhs), std::move(rhs));
    }
}



std::unique_ptr<Expr> Parser::ParseCallExpr(std::unique_ptr<Expr>&& fn, bool isSpawnedExpr)
{
    std::unique_ptr<Expr> cExpr;
    std::vector<std::unique_ptr<Expr>> args;
    std::unique_ptr<Expr> arg;

    mCurrentToken = mLexer.GetNextToken();


    while (mCurrentToken != Lexer::Token::RIGHT_PAREN)
    {
        
        arg.reset(ParseExpr().release());
        args.push_back(std::move(arg));

        if (mCurrentToken == Lexer::Token::RIGHT_PAREN) 
        {
            break;
        }
        else if (mCurrentToken == Lexer::Token::COMMA)  
        {
            mCurrentToken = mLexer.GetNextToken();
        }
        else if (mCurrentToken == Lexer::Token::SEMI_COLON) 
        {
            cExpr.reset();
            return std::move(cExpr);
        }
        else 
        {
            cExpr.reset();
            return std::move(cExpr);
        }
    }

    mCurrentToken = mLexer.GetNextToken();

    cExpr.reset(new CallExpr(fn->GetName(), std::move(args)));

    if (isSpawnedExpr)
        cExpr.reset(new SpawnExpr(std::move(cExpr)));
    
    return cExpr;
}

std::unique_ptr<CompoundStmt> Parser::ParseCompoundStmt()
{
    if (mCurrentToken != Lexer::Token::LEFT_BRACE)
    {
        return nullptr;
    }

    auto cStmt = std::make_unique<CompoundStmt>();

    mCurrentToken = mLexer.GetNextToken();
    std::unique_ptr<ASTNode> node;

    while ((mCurrentToken != Lexer::Token::RIGHT_BRACE) && (mCurrentToken != Lexer::Token::TOK_EOF))
    {
        switch (mCurrentToken)
        {
        case Lexer::Token::VAR:
            node.reset(ParseVarDecl().release());
            break;
        case Lexer::Token::IDENTIFIER:
            node.reset(ParseExpr().release());
            break;
        case Lexer::Token::RETURN:
            node.reset(ParseReturnStmt().release());
            break;
        case Lexer::Token::FUNCTION:
            while ((mCurrentToken != Lexer::Token::RIGHT_BRACE) && (mCurrentToken != Lexer::Token::TOK_EOF))
                mCurrentToken = mLexer.GetNextToken();
            break;
        default:
            break;
        }

        cStmt->AddStatement(std::move(node));

        mCurrentToken = mLexer.GetNextToken();
    }

    if (mCurrentToken != Lexer::Token::RIGHT_BRACE)
    {
        return std::move(cStmt);
    }

    return cStmt;
}

std::unique_ptr<ReturnStmt> Parser::ParseReturnStmt()
{
    auto rStmt = std::make_unique<ReturnStmt>(mLexer.GetCurrentLocation());

    mCurrentToken = mLexer.GetNextToken();
    rStmt->AddReturnValue(std::move(ParseExpr()));

    if (mCurrentToken != Lexer::Token::SEMI_COLON)
    
    return rStmt;
}