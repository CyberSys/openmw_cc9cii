#include "quickfileparser.hpp"

#include "skipparser.hpp"
#include "scanner.hpp"

Tes4Compiler::QuickFileParser::QuickFileParser (Compiler::ErrorHandler& errorHandler,
    const Compiler::Context& context, Compiler::Locals& locals)
: Parser (errorHandler, context), mDeclarationParser (errorHandler, context, locals)
{}

bool Tes4Compiler::QuickFileParser::parseName (const std::string& name, const Compiler::TokenLoc& loc,
    Scanner& scanner)
{
    SkipParser skip (getErrorHandler(), getContext());
    scanner.scan (skip);
    return true;
}

bool Tes4Compiler::QuickFileParser::parseKeyword (int keyword, const Compiler::TokenLoc& loc, Scanner& scanner)
{
    // scanning past Begin can take a long time, so we probably will need a different solution;
    // also this variable is not going to be used by any other script so if it isn't detected
    // there won't be any issues?
#if 1
    if (keyword == Scanner::K_begin || keyword == Scanner::K_end)
        return false;
#else
    if (keyword == Scanner::K_end)
        return false; // stop scanning

    // AldosOthran script (0003C2CC)
    //
    // Scriptname MS10AldosScript
    //
    // begin OnDeath Player
    // short Doonce <--------- this declaration comes after begin OnDeath Player block
    //
    if (keyword == Scanner::K_begin)
        return true; // continue scanning (hack to workaround AldosOthran script issue)
#endif

    if (keyword == Scanner::K_short || keyword == Scanner::K_long // SE06SCRIPT uses "int"
            || keyword == Scanner::K_float || keyword == Scanner::K_ref || keyword==Scanner::K_int)
    {
        mDeclarationParser.reset();
        scanner.putbackKeyword (keyword, loc);
        scanner.scan (mDeclarationParser);
        return true;
    }

    SkipParser skip (getErrorHandler(), getContext());
    scanner.scan (skip);
    return true;
}

bool Tes4Compiler::QuickFileParser::parseSpecial (int code, const Compiler::TokenLoc& loc, Scanner& scanner)
{
    if (code != Scanner::S_newline)
    {
        SkipParser skip (getErrorHandler(), getContext());
        scanner.scan (skip);
    }

    return true;
}

void Tes4Compiler::QuickFileParser::parseEOF (Scanner& scanner)
{

}
