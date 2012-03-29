/***************************************************************************
    File                 : muParserScript.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------

    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : Evaluate mathematical expressions using muParser

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "muParserScript.h"
#include "muParserScripting.h"
#include "Table.h"
#include "Matrix.h"
#include "Folder.h"

#include <QStringList>
#include <QApplication>
#include <QMessageBox>

#include <gsl/gsl_math.h>

using namespace mu;

muParserScript::muParserScript(ScriptingEnv *env,  const QString &name, QObject *context,
    bool checkMultilineCode)
: Script(env,name, Script::NonInteractive, context),
  d_warn_multiline_code(checkMultilineCode)
{
  variables.setAutoDelete(true);
  rvariables.setAutoDelete(true);

  parser.DefineConst("pi", M_PI);
  parser.DefineConst("Pi", M_PI);
  parser.DefineConst("PI", M_PI);
  parser.DefineConst("e", M_E);
  parser.DefineConst("E", M_E);

  for (const muParserScripting::mathFunction *i=muParserScripting::math_functions; i->name; i++)
    if (i->numargs == 1 && i->fun1 != NULL)
      parser.DefineFun(i->name, i->fun1);
    else if (i->numargs == 2 && i->fun2 != NULL)
      parser.DefineFun(i->name, i->fun2);
    else if (i->numargs == 3 && i->fun3 != NULL)
      parser.DefineFun(i->name, i->fun3);

  if (context->isA("Table")) {
    parser.DefineFun("col", mu_col, false);
    parser.DefineFun("cell", mu_tableCell);
    parser.DefineFun("tablecol", mu_tablecol, false);
  } else if (context->isA("Matrix"))
    parser.DefineFun("cell", mu_cell);

  rparser = parser;
  if (context->isA("Table") || context->isA("Matrix")){
    if (d_warn_multiline_code){
      QApplication::restoreOverrideCursor();
      QString mess = tr("Multiline expressions take much more time to evaluate! Do you want to continue anyways?");
      if (QMessageBox::Yes == QMessageBox::warning((QWidget*)context, tr("MantidPlot") + " - " + tr("Warning"), mess,
          QMessageBox::Yes, QMessageBox::Cancel)){
        parser.SetVarFactory(mu_addVariable);
        rparser.SetVarFactory(mu_addVariableR);
      }
    } else {
      parser.SetVarFactory(mu_addVariable);
      rparser.SetVarFactory(mu_addVariableR);
    }
  } else {
    parser.SetVarFactory(mu_addVariable);
    rparser.SetVarFactory(mu_addVariableR);
  }
}

double muParserScript::col(const QString &arg)
{
  if (!context()->isA("Table"))
    throw Parser::exception_type(tr("col() works only on tables!").ascii());
  QStringList items;
  QString item = "";

  for (int i=0; i < arg.size(); i++) {
    if (arg[i] == '"') {
      item += "\"";
      for (i++; i < arg.size() && arg[i] != '"'; i++)
        if (arg[i] == '\\') {
          item += "\\";
          item += arg[++i];
        } else
          item += arg[i];
      item += "\"";
    } else if (arg[i] == ',') {
      items << item;
      item = "";
    } else
      item += arg[i];
  }
  items << item;

  Table *table = (Table*) const_cast<QObject*>(context());
  int col, row;
  Parser local_parser(rparser);
  if (items[0].startsWith("\"") && items[0].endsWith("\"")) {
    col = table->colNames().findIndex(items[0].mid(1,items[0].length()-2));
    if (col<0)
      throw Parser::exception_type(tr("There's no column named %1 in table %2!").
          arg(items[0]).arg(context()->name()).ascii());
  } else {// for backwards compatibility
    col = table->colNames().findIndex(items[0]);
    if (col<0) {
      local_parser.SetExpr(items[0].ascii());
      col = qRound(local_parser.Eval()) - 1;
    }
  }

  if (items.count() == 2)
  {
    local_parser.SetExpr(items[1].ascii());
    row = qRound(local_parser.Eval()) - 1;
  } else if (variables["i"])
    row = (int) *(variables["i"]) - 1;
  else
    return 0;
  rvariables.clear();
  if (row < 0 || row >= table->numRows())
    throw Parser::exception_type(tr("There's no row %1 in table %2!").
        arg(row+1).arg(context()->name()).ascii());
  if (col < 0 || col >= table->numCols())
    throw Parser::exception_type(tr("There's no column %1 in table %2!").
        arg(col+1).arg(context()->name()).ascii());
  if (table->text(row, col).isEmpty())
    throw new EmptySourceError();
  else
    return table->cell(row,col);
}

double muParserScript::tablecol(const QString &arg)
{
  if (!context()->isA("Table"))
    throw Parser::exception_type(tr("tablecol() works only on tables!").ascii());
  QStringList items;
  QString item = "";
  for (int i=0; i < arg.size(); i++) {
    if (arg[i] == '"') {
      item += "\"";
      for (i++; i < arg.size() && arg[i] != '"'; i++)
        if (arg[i] == '\\') {
          item += "\\";
          item += arg[++i];
        } else
          item += arg[i];
      item += "\"";
    } else if (arg[i] == ',') {
      items << item;
      item = "";
    } else
      item += arg[i];
  }
  items << item;

  Table *this_table = (Table*) const_cast<QObject*>(context());
  int col, row;
  Parser local_parser(rparser);
  if (items.count() != 2)
    throw Parser::exception_type(tr("tablecol: wrong number of arguments (need 2, got %1)").arg(items.count()).ascii());
  if (!items[0].startsWith("\"") || !items[0].endsWith("\""))
    throw Parser::exception_type(tr("tablecol: first argument must be a string (table name)").ascii());
  Table *target_table = this_table->folder()->rootFolder()->table(items[0].mid(1,items[0].length()-2));
  if (!target_table)
    throw Parser::exception_type(tr("Couldn't find a table named %1.").arg(items[0]).ascii());
  if (items[1].startsWith("\"") && items[1].endsWith("\"")) {
    col = target_table->colNames().findIndex(items[1].mid(1,items[1].length()-2));
    if (col<0)
      throw Parser::exception_type(tr("There's no column named %1 in table %2!").
          arg(items[1]).arg(target_table->name()).ascii());
  } else {
    local_parser.SetExpr(items[1].ascii());
    col = qRound(local_parser.Eval()) - 1;
  }
  if (variables["i"])
    row = (int) *(variables["i"]) - 1;
  else
    row = -1;
  rvariables.clear();
  if (row < 0 || row >= target_table->numRows())
    throw Parser::exception_type(tr("There's no row %1 in table %2!").
        arg(row+1).arg(target_table->name()).ascii());
  if (col < 0 || col >= target_table->numCols())
    throw Parser::exception_type(tr("There's no column %1 in table %2!").
        arg(col+1).arg(target_table->name()).ascii());
  if (target_table->text(row,col).isEmpty())
    throw new EmptySourceError();
  else
    return target_table->cell(row,col);
}

double muParserScript::cell(int row, int col)
{
  if (!context()->isA("Matrix"))
    throw Parser::exception_type(tr("cell() works only on tables and matrices!").ascii());
  Matrix *matrix = (Matrix*) const_cast<QObject*>(context());
  if (row < 1 || row > matrix->numRows())
    throw Parser::exception_type(tr("There's no row %1 in matrix %2!").
        arg(row).arg(context()->name()).ascii());
  if (col < 1 || col > matrix->numCols())
    throw Parser::exception_type(tr("There's no column %1 in matrix %2!").
        arg(col).arg(context()->name()).ascii());
  if (matrix->text(row-1,col-1).isEmpty())
    throw new EmptySourceError();
  else
    return matrix->cell(row-1,col-1);
}

double muParserScript::tableCell(int col, int row)
{
  if (!context()->isA("Table"))
    throw Parser::exception_type(tr("cell() works only on tables and matrices!").ascii());
  Table *table = (Table*) const_cast<QObject*>(context());
  if (row < 1 || row > table->numRows())
    throw Parser::exception_type(tr("There's no row %1 in table %2!").
        arg(row).arg(context()->name()).ascii());
  if (col < 1 || col > table->numCols())
    throw Parser::exception_type(tr("There's no column %1 in table %2!").
        arg(col).arg(context()->name()).ascii());
  if (table->text(row-1,col-1).isEmpty())
    throw new EmptySourceError();
  else
    return table->cell(row-1,col-1);
}

double *muParserScript::addVariable(const char *name)
{
  double *valptr = new double;
  if (!valptr)
    throw Parser::exception_type(tr("Out of memory").ascii());
  *valptr = 0;
  variables.insert(name, valptr);
  rparser.DefineVar(name, valptr);
  return valptr;
}

double *muParserScript::addVariableR(const char *name)
{
  double *valptr = new double;
  if (!valptr)
    throw Parser::exception_type(tr("Out of memory").ascii());
  *valptr = 0;
  rvariables.insert(name, valptr);
  return valptr;
}

double* muParserScript::defineVariable(const char *name, double val)
{
  double *valptr = variables[name];
  if (!valptr)
  {
    valptr = new double;
    if (!valptr)
    {
      emit error(tr("Out of memory"), this->name(), 0);
      return 0;
    }
    try {
      parser.DefineVar(name, valptr);
      rparser.DefineVar(name, valptr);
      variables.insert(name, valptr);
    } catch (mu::ParserError &e) {
      delete valptr;
      emit error(QString(e.GetMsg().c_str()), this->name(), 0);
      return valptr;
    }
  }
  *valptr = val;
  return valptr;
}

bool muParserScript::setDouble(double val, const char *name)
{
  double *valptr = variables[name];
  if (!valptr)
  {
    valptr = new double;
    if (!valptr)
    {
      emit error(tr("Out of memory"), this->name(), 0);
      return false;
    }
    try {
      parser.DefineVar(name, valptr);
      rparser.DefineVar(name, valptr);
      variables.insert(name, valptr);
    } catch (mu::ParserError &e) {
      delete valptr;
      emit error(QString(e.GetMsg().c_str()), this->name(), 0);
      return false;
    }
  }
  *valptr = val;
  return  true;
}

bool muParserScript::setInt(int val, const char *name)
{
  return setDouble((double) val, name);
}

bool muParserScript::setQObject(QObject*, const char*)
{
  return false;
}

QString muParserScript::compileColArg(const QString &in)
{
  QString out = "\"";
  for (int i=0; i < in.size(); i++)
    if (in[i] == 'c' && in.mid(i,4)=="col(") {
      out += "col(";
      QString arg = "";
      int paren = 1;
      for (i+=4; i < in.size() && paren > 0; i++)
        if (in[i] == '"') {
          arg += "\"";
          for (i++; i < in.size() && in[i] != '"'; i++)
            if (in[i] == '\\') {
              arg += "\\";
              arg += in[++i];
            } else
              arg += in[i];
          arg += "\"";
        } else if (in[i] == '(') {
          paren++;
          arg += "(";
        } else if (in[i] == ')') {
          paren--;
          if(paren>0) arg += ")";
        }
        else
          arg += in[i];
      out += compileColArg(arg).replace('"',"\\\"");
      out += ")";
      i--;
    }
    else if (in[i] == '\\')
      out += "\\\\";
    else if (in[i] == '"')
      out += "\\\"";
    else
      out += in[i];
  return out + "\"";
}


QString muParserScript::evalSingleLineToString(const QLocale& locale, char f, int prec)
{
  double val = 0.0;
  try {
    val = parser.Eval();
  } catch (EmptySourceError*) {
    return "";
  } catch (ParserError &) {
    return "";
  }
  return locale.toString(val, f, prec);
}

double muParserScript::evalSingleLine()
{
  double val = 0.0;
  try {
    val = parser.Eval();
  } catch (EmptySourceError *) {
    return GSL_NAN;
  } catch (ParserError &) {
    return GSL_NAN;
  }
  return val;
}

muParserScript *muParserScript::current = NULL;

bool muParserScript::compile(const QString & code)
{
  muCode.clear();
  QString muCodeLine = "";
  for (int i=0; i < code.size(); i++)
    if (code[i] == 'c' && code.mid(i,4)=="col(") {
      muCodeLine += "col(";
      QString arg = "";
      int paren = 1;
      for (i+=4; i < code.size() && paren > 0; i++)
        if (code[i] == '"') {
          arg += "\"";
          for (i++; code[i] != '"' && i < code.size(); i++)
            if (code[i] == '\\') {
              arg += "\\";
              arg += code[++i];
            } else
              arg += code[i];
          arg += "\"";
        } else if (code[i] == '(') {
          paren++;
          arg += "(";
        } else if (code[i] == ')') {
          paren--;
          if(paren>0) arg += ")";
        }
        else
          arg += code[i];
      muCodeLine += compileColArg(arg);
      muCodeLine += ")";
      i--;
    } else if (code[i] == '#')
      for (i++; code[i] != '\n' && i < code.size(); i++){;}
    else if (code[i] == '\n') {
      muCodeLine = muCodeLine.stripWhiteSpace();
      if (!muCodeLine.isEmpty())
        muCode += muCodeLine;
      muCodeLine = "";
    } else
      muCodeLine += code[i];
  muCodeLine = muCodeLine.stripWhiteSpace();
  if (!muCodeLine.isEmpty())
    muCode += muCodeLine;

  if (muCode.size() == 1){
    current = this;
    parser.SetExpr(muCode[0].ascii());
    try {
      parser.Eval();
    } catch (EmptySourceError*) {
      QApplication::restoreOverrideCursor();
      return false;
    } catch (mu::ParserError &e) {
      if (e.GetCode() != ecVAL_EXPECTED){
        QApplication::restoreOverrideCursor();
        emit error(e.GetMsg().c_str(), name(), 0);
        return false;
      }
    }
  }
  return true;
}

QVariant muParserScript::evaluate(const QString & code)
{
  if (!compile(code))
    return QVariant();
  double val = 0.0;
  try {
    current = this;
    for (QStringList::iterator i=muCode.begin(); i != muCode.end(); ++i) {
      parser.SetExpr(i->ascii());
      val = parser.Eval();
    }
  } catch (EmptySourceError *) {
    return QVariant("");
  } catch (ParserError &e) {
    emit error(e.GetMsg().c_str(), name(), 0);
    return QVariant();
  }
  return QVariant(val);
}

bool muParserScript::execute(const QString & code)
{
  if (!compile(code))
    return false;
  try {
    current = this;
    for (QStringList::iterator i=muCode.begin(); i != muCode.end(); ++i) {
      parser.SetExpr(i->ascii());
      parser.Eval();
    }
  } catch (EmptySourceError *) {
    return true;
  } catch (mu::ParserError &e) {
    emit error(e.GetMsg().c_str(), name(), 0);
    return false;
  }
  return true;
}

/**
 * Execute the script in a seprate thread
 */
QFuture<bool> muParserScript::executeAsync(const QString &)
{
  throw std::runtime_error("muParser does not support asynchronous execution");
}
