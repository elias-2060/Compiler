#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setStyleSheet("background-color: #4d3df5; color: white; border: none; border-radius: 5px; padding: 10px 20px; font-size: 17px;");
    ui->textEdit->setStyleSheet("color: white; border: 1px solid blue; border-radius: 10px; font-size: 14px;");
    ui->textBrowser->setStyleSheet("color: green; border: 1px solid green; border-radius: 10px; font-size: 14px;");
    ui->textBrowser_2->setStyleSheet("color: red; border: 1px solid red; border-radius: 10px; font-size: 14px;");
    ui->pushButton->setCursor(Qt::PointingHandCursor);
    ui->textEdit->setPlaceholderText("Write your code here");
    ui->textBrowser->setReadOnly(true);
    ui->textBrowser_2->setReadOnly(true);
    ui->textEdit->setFocus();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked(){
    // Get the text from the text editor
    QString text = ui->textEdit->toPlainText();
    string sourceCode = text.toStdString();

    // Create lexer and convert the input into tokens
    Lexer lexer(sourceCode);

    // Check for lexical errors
    string syntaxErrors = lexer.getErrors();
    if (!syntaxErrors.empty()){
        ui->textBrowser_2->setText(QString::fromStdString(syntaxErrors));
        ui->textBrowser->setText("");
        return;
    }

    // Get the tokens
    queue<Token> tokens = lexer.getTokens();

    // Create parser
    Parser parser("CFG.json");
    //parser.printTable();

    // Parse de tokens
    bool accept = parser.parse(tokens);
    if (!accept){
        ui->textBrowser_2->setText(QString::fromStdString("Syntax error"));
        ui->textBrowser->setText("");
        return;
    }

    // Get the CST
    Node* cst = parser.getCST();
    // Print the CST in dot formaat
    parser.printTree(cst, "CST.dot");

    // Create the AST
    Node* ast = parser.getAST();

    // Print the AST in dot formaat
    parser.printTree(ast, "AST.dot");

    // Initialize the symbol table and check for errors
    parser.initSymbolTable(ast);
    //parser.printSymbolTable();

    // Check for semantic errors
    string errors = parser.getErrors();
    if (!errors.empty()){
        ui->textBrowser_2->setText(QString::fromStdString(errors));
        ui->textBrowser->setText("");
        return;
    }

    // Do constant folding
    parser.constantFolding(ast);

    // Print the AST after constant folding in dot formaat
    parser.printTree(ast, "AST2.dot");

    // Create the compiler
    Compiler compiler(parser.getSymbolTable(), ast);

    // Convert the AST to mips
    compiler.convertToMips();

    // Print the mips code
    string mipsCode = compiler.getMipsCode();
    ui->textBrowser->setText(QString::fromStdString(mipsCode));
    ui->textBrowser_2->setText("");

}