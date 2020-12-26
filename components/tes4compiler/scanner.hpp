#ifndef TES4COMPILER_SCANNER_H_INCLUDED
#define TES4COMPILER_SCANNER_H_INCLUDED

#include <string>
#include <iosfwd>
#include <vector>

#include "../compiler/tokenloc.hpp"

namespace Compiler
{
    class ErrorHandler;
    class Extensions;
}

namespace Tes4Compiler
{
    class Parser;
    //class TES4Extensions;

    /// \brief Scanner
    ///
    /// This class translate a char-stream to a token stream (delivered via
    /// parser-callbacks).

    class Scanner
    {
            enum putback_type
            {
                Putback_None, Putback_Special, Putback_Integer, Putback_Float,
                Putback_Name, Putback_Keyword
            };

            Compiler::ErrorHandler& mErrorHandler;
            Compiler::TokenLoc mLoc;
            Compiler::TokenLoc mPrevLoc;
            std::istream& mStream;
            const Compiler::Extensions *mExtensions;
            putback_type mPutback;
            int mPutbackCode;
            int mPutbackInteger;
            float mPutbackFloat;
            std::string mPutbackName;
            Compiler::TokenLoc mPutbackLoc;

        public:

            enum keyword
            {
                K_begin, K_end,
                K_short, K_long, K_float, K_ref,
                K_if, K_endif, K_else, K_elseif,
                K_while, K_endwhile,
                K_return,
                K_messagebox, K_message,
                K_set, K_to,
                K_getsquareroot,
                K_menumode,
                K_random,
                K_startscript, K_stopscript, K_scriptrunning,
                K_getdistance,
                K_getsecondspassed,
                K_enable, K_disable, K_getdisabled,
                K_scriptname, K_scn
            };

            enum special
            {
                S_newline,
                S_open, S_close,
                S_cmpEQ, S_cmpNE, S_cmpLT, S_cmpLE, S_cmpGT, S_cmpGE,
                S_plus, S_minus, S_mult, S_div,
                S_and, S_or,
                S_comma,
                S_ref,
                S_member // FIXME: not present in TES4 scripts?
            };

        private:

        // not implemented

            Scanner (const Scanner&);
            Scanner& operator= (const Scanner&);

            bool get (char& c);

            void putback (char c);

            bool scanToken (Parser& parser);

            bool scanInt (char c, Parser& parser, bool& cont);

            bool scanFloat (const std::string& intValue, Parser& parser, bool& cont);

            bool scanName (char c, Parser& parser, bool& cont);

            /// \param name May contain the start of the name (one or more characters)
            bool scanName (std::string& name);

            bool scanSpecial (char c, Parser& parser, bool& cont);

            bool isStringCharacter (char c, bool lookAhead = true);

            static bool isWhitespace (char c);

        public:

            Scanner (Compiler::ErrorHandler& errorHandler, std::istream& inputStream,
                const Compiler::Extensions *extensions = 0);
            ///< constructor

            void scan (Parser& parser);
            ///< Scan a token and deliver it to the parser.

            void putbackSpecial (int code, const Compiler::TokenLoc& loc);
            ///< put back a special token

            void putbackInt (int value, const Compiler::TokenLoc& loc);
            ///< put back an integer token

            void putbackFloat (float value, const Compiler::TokenLoc& loc);
            ///< put back a float token

            void putbackName (const std::string& name, const Compiler::TokenLoc& loc);
            ///< put back a name toekn

            void putbackKeyword (int keyword, const Compiler::TokenLoc& loc);
            ///< put back a keyword token

            void listKeywords (std::vector<std::string>& keywords);
            ///< Append all known keywords to \a kaywords.
    };
}

#endif