#ifndef MATRIX_H
#define MATRIX_H

#include "characters.h"
#include <avr/pgmspace.h>

// LED on: anode HIGH cathode LOW
// default configuration (nothing on) is anode LOW and cathode HIGH
// anodes will be switched on and off very rapidly and cathode state determines if the LED is on
class Matrix {
    private:
        // pins
        short m_numAnodes;
        short m_numCathodes;
        short *m_anodePins;
        short *m_cathodePins;

        // used for string scrolling
        unsigned long m_prevTime = 0;
        short m_scrollDelay = 0;
        short m_scrollPosition = 0;
        short m_firstIndex = 0;
        String m_scrollString = "";
        bool m_isFinished = true;

        // matrix represented as 2d array
        bool **m_matrix;

        // range check for matrix access
        bool inRange(short x, short y) {
            return !(x < 0 || x >= m_numAnodes || y < 0 || y >= m_numCathodes);
        }

    public:
        void init(short numAnodes, short numCathodes,
                  short* anodePins, short* cathodePins) {

            m_numAnodes = numAnodes;
            m_numCathodes = numCathodes;
            m_anodePins = anodePins;
            m_cathodePins = cathodePins;

            // anode outputs
            for (short anode = 0; anode < numAnodes; anode++) {
                pinMode(m_anodePins[anode], OUTPUT);
                digitalWrite(m_anodePins[anode], LOW);
            }

            // cathode outputs
            for (short cathode = 0; cathode < numCathodes; cathode++) {
                pinMode(m_cathodePins[cathode], OUTPUT);
                digitalWrite(m_cathodePins[cathode], HIGH);
            }

            // create matrix and zero out
            m_matrix = new bool*[numAnodes];
            for (short anode = 0; anode < numAnodes; anode++)
                m_matrix[anode] = new bool[numCathodes];

            clear();
        }

        // returns true if the matrix has finished displaying
        bool isFinished()
        {
            return m_isFinished;
        }

        // display a string by side-scrolling
        void displayString(const String &string, short scrollDelay)
        {
            m_scrollString = string;
            m_scrollDelay = scrollDelay;
            m_scrollPosition = 8;
            m_firstIndex = 0;
            m_isFinished = false;
        }

        // set character at x,y
        // characters are sized 5x8
        void setCharacter(char c, short px = 0, short py = 0) {
            // character range
            if (c < ' ' || c > '~') return;

            for (short x = 0; x < 5; x++) {
                for (short y = 0; y < 8; y++) {
                    // all of the character data is stored in program space because it's a lot
                    // could need some optimization, but eh, it works
                    set(x + px - 1, y + py + 1, pgm_read_byte(&(characters[c - ' '][y][x])));
                }
            }
        }

        // set pins based on the matrix grid
        void updatePins()
        {
            for (short anode = 0; anode < m_numAnodes; anode++) {
                digitalWrite(m_anodePins[anode], HIGH);
                for (short cathode = 0; cathode < m_numCathodes; cathode++) {
                    if (m_matrix[anode][cathode])
                        digitalWrite(m_cathodePins[cathode], LOW);
                    else
                        digitalWrite(m_cathodePins[cathode], HIGH);
                }

                delay(2);
                digitalWrite(m_anodePins[anode], LOW);
            }
        }

        // set individial LEDs
        void set(short x, short y, bool state)
        {
            if (!inRange(x, y)) return;
            m_matrix[x][y] = state;
        }

        // update scroll and pin states
        void updateScroll() {
            // scrolling has finished if the first character index is not valid anymore
            // (including zero-terminator)
            if (m_firstIndex == m_scrollString.length()) m_isFinished = true;

            // if the matrix finished last frame and no new string was set, we can skip
            if (m_isFinished) return;

            // update scrolling
            if (m_scrollString != "") {
                unsigned long currentTime = millis();

                if (currentTime - m_prevTime > m_scrollDelay) {
                    clear();
                    m_prevTime = currentTime;

                    // draw second and third char first (if existing)
                    if ((m_scrollString.length() - m_firstIndex) > 1) {
                        setCharacter(m_scrollString[m_firstIndex + 1], m_scrollPosition + 6);
                        if ((m_scrollString.length() - m_firstIndex) > 2)
                            setCharacter(m_scrollString[m_firstIndex + 2], m_scrollPosition + 12);
                    }

                    // if first char is off-screen , remove it and update scroll position
                    if (m_scrollPosition <= -5) {
                        m_firstIndex++;
                        m_scrollPosition += 5;
                    } else {
                        setCharacter(m_scrollString[m_firstIndex], m_scrollPosition);
                        m_scrollPosition--;
                    }
                }
            }

            updatePins();
        }

        // zero out the matrix
        void clear() {
            // does not work for some reason, so who cares...
            // memset(m_matrix, 0, sizeof(bool) * m_numAnodes * m_numCathodes);

            for (short x = 0; x < m_numAnodes; x++)
                for (short y = 0; y < m_numCathodes; y++)
                    m_matrix[x][y] = false;
        }
};

#endif