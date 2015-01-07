#include <iostream>
#include <string>
#define WIN32_EXTRA_LEAN
#include <windows.h>

namespace
{
    int const BEEP_FREQUENCY    = 700;
    int const UNIT              = 100;
    int const DOT_LENGTH        = 1 * UNIT;
    int const DASH_LENGTH       = 3 * UNIT;
    int const INTER_ELEMENT_GAP = 1 * UNIT;
    int const INTER_LETTER_GAP  = 3 * UNIT;
    int const INTER_WORD_GAP    = 7 * UNIT;
}

//=================================================================================================
void delay(int ms)
{
    Sleep(ms);
}

//=================================================================================================
void Dot()
{
    Beep(BEEP_FREQUENCY, DOT_LENGTH);
    /*
    std::cout << "." << std::flush;
    delay(DOT_LENGTH);
    //std::cout << " " << std::flush;
    */
}

//=================================================================================================
void Dash()
{
    Beep(BEEP_FREQUENCY, DASH_LENGTH);
    /*
    std::cout << "-" << std::flush;
    delay(DASH_LENGTH);
    //std::cout << " " << std::flush;
    */
}

//=================================================================================================
void Play(char const* morse)
{
    while (*morse)
    {
        if (*morse == '.') { Dot();  }
        if (*morse == '-') { Dash(); }
        delay(INTER_ELEMENT_GAP);
        ++morse;
    }
    delay(INTER_LETTER_GAP - INTER_ELEMENT_GAP);
    //std::cout << " " << std::flush;
}

//=================================================================================================
void TranslateLetter(char c)
{
    switch (tolower(c))
    {
    case ' ': delay(INTER_WORD_GAP - INTER_LETTER_GAP - INTER_ELEMENT_GAP); /*std::cout << " ";*/ break;
    case 'a': Play(".-");    break;
    case 'b': Play("-...");  break;
    case 'c': Play("-.-.");  break;
    case 'd': Play("-..");   break;
    case 'e': Play(".");     break;
    case 'f': Play("..-.");  break;
    case 'g': Play("--.");   break;
    case 'h': Play("....");  break;
    case 'i': Play("..");    break;
    case 'j': Play(".---");  break;
    case 'k': Play("-.-");   break;
    case 'l': Play(".-..");  break;
    case 'm': Play("--");    break;
    case 'n': Play("-.");    break;
    case 'o': Play("---");   break;
    case 'p': Play(".--.");  break;
    case 'q': Play("--.-");  break;
    case 'r': Play(".-.");   break;
    case 's': Play("...");   break;
    case 't': Play("-");     break;
    case 'u': Play("..-");   break;
    case 'v': Play("...-");  break;
    case 'w': Play(".--");   break;
    case 'x': Play("-..-");  break;
    case 'y': Play("-.--");  break;
    case 'z': Play("--..");  break;
    case '1': Play(".----"); break;
    case '2': Play("..---"); break;
    case '3': Play("...--"); break;
    case '4': Play("....-"); break;
    case '5': Play("....."); break;
    case '6': Play("-...."); break;
    case '7': Play("--..."); break;
    case '8': Play("---.."); break;
    case '9': Play("----."); break;
    case '0': Play("-----"); break;
    default:  /*Play("................")*/ ;
    }
}

//=================================================================================================
void Translate(char const* s)
{
    while (*s)
    {
        TranslateLetter(*s++);
    }
}

//=================================================================================================
int main(int argc, char** argv)
{
    if (argc == 2)
    {
        char* s = argv[1];
        Translate(s);
    }
    else
    {
        while (true)
        {
            std::string phrase;
            std::cin >> phrase;
            if (phrase.empty()) { break; }
            Translate(phrase.c_str());
            std::cout << "\n\n";
        }
    }
    return 0;
}

