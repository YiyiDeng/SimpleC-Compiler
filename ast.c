// TODO : 符号表的作用域限定

#include "ast.h"

int i;
struct ast *newast(char*  name,int num,...)//抽象语法树建立
{
    va_list valist; //定义变长参数列表
    struct ast *a=(struct ast*)malloc(sizeof(struct ast));//新生成的父节点
    struct ast *temp=(struct ast*)malloc(sizeof(struct ast));
    if(!a)
    {
        yyerror("out of space");
        exit(0);
    }
    a->name=name;//语法单元名字
    va_start(valist,num);//初始化变长参数为num后的参数

    if(num>0)//num>0为非终结符：变长参数均为语法树结点，孩子兄弟表示法
    {
        temp=va_arg(valist, struct ast*);//取变长参数列表中的第一个结点设为a的左孩子
        a->l=temp;
        a->line=temp->line;//父节点a的行号等于左孩子的行号
        if(num==1)//只有一个孩子
        {
            a->content=temp->content;//父节点的语义值等于左孩子的语义值
            a->tag=temp->tag;
        }
        else //可以规约到a的语法单元>=2
        {
            for(i=0; i<num-1; ++i)//取变长参数列表中的剩余结点，依次设置成兄弟结点
            {
                temp->r=va_arg(valist,struct ast*);
                temp = temp->r;
            }
        }
    }
    else //num==0为终结符或产生空的语法单元：第1个变长参数表示行号，产生空的语法单元行号为-1。
    {
        int t=va_arg(valist, int); //取第1个变长参数
        a->line=t;
        if(!strcmp(a->name,"INTEGER"))//函数符号表头指针a->name,"INTEGER"))
        {
            a->type="int";
            a->value=atof(yytext);
        }
        else if(!strcmp(a->name,"FLOAT"))
        {
            a->type="float";
            a->value=atof(yytext);
        }
        else if(!strcmp(a->name,"OCT"))
        {
            a->type="int";

            char * p = yytext;
            //int str_len(char *s);
            int length = strlen(p);
            int n=0, k=1;
            
            for (int i = length-1; i > 0; i--)
            {
                n += k*(p[i]-48);
                k *= 8;
            }
            a->value=n;
        }
        else if(!strcmp(a->name,"HEX"))
        {
            a->type="int";

            char * p = yytext;
            int length = strlen(p);
            int n=0, k=1;

            for (int i = length-1; i > 1; i--)
            {
                if(p[i]>=48 && p[i]<=57) // 0-9
                    n += k*(p[i]-48);
                if(p[i]>=65 && p[i]<=90) //A-Z
                    n += k*(p[i]-55);
                if(p[i]>=97 && p[i]<=122) { // a-z
                    n += (p[i]-87)*k;
                }   
                k *= 16;
            }
            a->value=n;
        }
        else
        {
            char* s;
            s=(char*)malloc(sizeof(char* )*40);
            strcpy(s,yytext);//存储词法单元的语义值
            a->content=s;
        }
    }
    return a;
}

void eval(struct ast *a,int level)//先序遍历抽象语法树
{
    if(a!=NULL)
    {
        for(i=0; i<level; ++i)//孩子结点相对父节点缩进2个空格
            printf("  ");
        if(a->line!=-1)  //产生空的语法单元不需要打印信息
        {
            printf("%s ",a->name);//打印语法单元名字，ID/TYPE/INTEGER要打印yytext的值
            if((!strcmp(a->name,"ID"))||(!strcmp(a->name,"TYPE")))printf(":%s ",a->content);
            else if(!strcmp(a->name,"INTEGER")||(!strcmp(a->name,"OCT"))||(!strcmp(a->name,"HEX")))printf(":%0.0f",a->value);
            else
                printf("(%d)",a->line);
        }
        printf("\n");
        eval(a->l,level+1);//遍历左子树
        eval(a->r,level);//遍历右子树
    }
}

/*------ST1: Variable------*/

// add a variable in ST1
void 
newvar(int num,...)
{
    
    va_list valist; // define a variable-length argument list
    // create a parent node and pass the synthesized attribute
    struct var *a=(struct var*)malloc(sizeof(struct var));
    struct ast *temp=(struct ast*)malloc(sizeof(struct ast));
    va_start(valist,num);// initialize valist: arguments after num
    temp=va_arg(valist, struct ast*);// 1st node in valist
    a->type=temp->content;
    temp=va_arg(valist, struct ast*);// 2nd node in valist
    a->name=temp->content;
    vartail->next=a;
    vartail=a;
    a->next = NULL;
}
// check if the variable has already defined
int 
existvar(struct ast* temp)
{
    struct var *p = (struct var*)malloc(sizeof(struct var*));
    p=varhead->next;
    int flag=0;
    while(p!=NULL)
    {
        if(strcmp(p->name,temp->content)==0)
        {
            flag=1; 
            return 1;
        }
        p=p->next;
    }
    if(!flag)
    {
        return 0;
    }
    return 0;
}
// search for its type
char * 
typevar(struct ast* temp)
{
    struct var* p=(struct var*)malloc(sizeof(struct var*));
    p=varhead->next;
    while(p!=NULL)
    {
        if(strcmp(p->name,temp->content)==0)
            return p->type;
        p=p->next;
    }
    return 0;
}

/*------ST2: Function------*/

void 
newfunc(int num,...)
{
    va_list valist;
    struct ast *temp=(struct ast*)malloc(sizeof(struct ast));
    va_start(valist,num);
    switch(num)
    {
        case 1:
            functail->pnum+=1;// number of arguments +1
            break;
        case 2://记录函数名
            temp=va_arg(valist, struct ast*);//取变长参数列表中的第1个结点
            functail->name = temp->content;
            break;
        case 3://记录实际返回值
            temp=va_arg(valist, struct ast*);//取变长参数列表中的第1个结点
            functail->rtype=temp->type;
            break;
        default://记录函数类型,返回类型不匹配则报出错误
            rpnum=0;//将实参个数清0
            temp=va_arg(valist, struct ast*);//取变长参数列表中的第1个结点，即函数定义的返回类型
            if(functail->rtype!=NULL)
            {
                //实际返回类型和函数定义的返回类型比较
                if(strcmp(temp->content,functail->rtype))
                    printf("Error type 8 at Line %d:Type mismatched for return.\n",yylineno);
            }
            functail->type=temp->type;
            functail->tag=1;//标志为已定义
            struct func *a=(struct func*)malloc(sizeof(struct func));
            functail->next=a;//尾指针指向下一个空结点
            functail=a;
            break;
    }
}

int 
existfunc(struct ast* tp)
{
    struct func* p=(struct func*)malloc(sizeof(struct func*));
    p=funchead->next;
    
    while(p!=NULL&&p->name!=NULL&&p->tag==1)
    {
        if(!strcmp(p->name,tp->content)) 
            return 1;
        p=p->next;
    }
   return 0;
}

char * 
typefunc(struct ast*tp)
{
    struct func* p=(struct func*)malloc(sizeof(struct func*));
    p=funchead->next;
    while(p!=NULL)
    {
        if(!strcmp(p->name,tp->content))
            return p->type;
        p=p->next;
    }
    return 0;
}
// search the number of arguments
int 
pnumfunc(struct ast*tp)
{
    struct func* p=(struct func*)malloc(sizeof(struct func*));
    p=funchead;
    while(p->next!=NULL)
    {
        p=p->next;
        if(!strcmp(p->name,tp->content))
            return p->pnum;
    }
    return p->pnum;
}

/*------ST3: array------*/

void 
newarray(int num,...)//1)创建数组符号表
{
    va_list valist; //定义变长参数列表
    struct array *a=(struct array*)malloc(sizeof(struct array));//新生成的父节点
    struct ast *temp=(struct ast*)malloc(sizeof(struct ast));
    va_start(valist,num);//初始化变长参数为num后的参数
    temp=va_arg(valist, struct ast*);//取变长参数列表中的第一个结点
    a->type=temp->content;
    temp=va_arg(valist, struct ast*);//取变长参数列表中的第二个结点
    a->name=temp->content;
    arraytail->next=a;
    arraytail=a;
    printf("arraytail:%s\n", arraytail->name);
}

int  
existarray(struct ast* tp)//2)查找数组是否已经定义,是返回1，否返回0
{
    struct array* p=(struct array*)malloc(sizeof(struct array*));
    p=arrayhead->next;
    int flag=0;
    while(p!=NULL)
    {
        if(strcmp(p->name,tp->content)==0)
        {
            flag=1;    //存在返回1
            printf("flag:%d\n", flag);
            return 1;
        }
        p=p->next;
    }
    if(!flag)
        return 0;//不存在返回0
    return 0;
}

char* 
typearray(struct ast* tp)//3)查找数组类型
{
    struct array* p=(struct array*)malloc(sizeof(struct array*));
    p=arrayhead->next;
    while(p!=NULL)
    {
        if(!strcmp(p->name,tp->content))
            return p->type;//返回数组类型
        p=p->next;
    }
    return 0;
}

/*====(4)结构体符号表的建立和查询================*/
void 
newstruc(int num,...)//1)创建结构体符号表
{
    printf("NOW ------ newstruc\n");
    va_list valist; //定义变长参数列表
    struct struc *a=(struct struc*)malloc(sizeof(struct struc));//新生成的父节点
    struct ast *temp=(struct ast*)malloc(sizeof(struct ast));
    va_start(valist,num);//初始化变长参数为num后的参数
    temp=va_arg(valist, struct ast*);//取变长参数列表中的第二个结点
    a->name=temp->content;
    structail->next=a;
    structail=a;
    printf("structail->name:%s\n", structail->name);
}

int  
existstruc(struct ast* tp)//2)查找结构体是否已经定义,是返回1，否返回0
{
    printf("existstruc:%s\n",tp->content);
    struct struc* p=(struct struc*)malloc(sizeof(struct struc*));
    p=struchead->next;
    while(p!=NULL)
    {
        if(strcmp(p->name,tp->content)==0)
        {
            printf("flag:%d\n",1);
            return 1;
        }
        p=p->next;
    }
    return 0;
}

int ifprvalue(struct ast* tp) {
    printf("ifprvalue: %s\n", tp->content);
    return 1;
}


void 
yyerror(char*s,...) //变长参数错误处理函数
{
    va_list ap;
    va_start(ap,s);
    fprintf(stderr,"%d:error:",yylineno);//错误行号
    vfprintf(stderr,s,ap);
    fprintf(stderr,"\n");
}


int main()
{
    varhead=(struct var*)malloc(sizeof(struct var));//变量符号表头指针
    vartail=varhead;//变量符号表尾指针
    vartail->next = NULL;

    funchead=(struct func*)malloc(sizeof(struct func));//函数符号表头指针
    functail=(struct func*)malloc(sizeof(struct func));//函数符号表头指针
    funchead->next=functail;//函数符号表尾指针
    functail->next = NULL;
    functail->pnum = 0;

    arrayhead=(struct array*)malloc(sizeof(struct array));//数组符号表头指针
    arraytail=arrayhead;
    arraytail->next = NULL;

    struchead=(struct struc*)malloc(sizeof(struct struc));//结构体符号表头指针
    structail=struchead;//结构体符号表尾指针
    structail->next = NULL;

    return yyparse(); //启动文法分析，调用词法分析
}
