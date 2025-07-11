# try to remember what this means

## the note

Let’s break that format string down once and for all:

css
Copy
Edit
%0[N]X (N is a number 1-9 maybe 0?)
Part	Meaning
%	Start of format sequence
0	Pad with zeroes (instead of spaces)
N	Minimum width (how many characters wide)
X	Print in Hexadecimal, uppercase (use x for lowercase)


```cpp
void canKick(const CAN_message_t &msg)
{
    // 1. Decide if it's an extended or standard frame
    char frameType = msg.flags.extended ? 'T' : 't';
    Serial.print(frameType); // T = extended, t = standard

    // 2. Print ID with proper width (3 hex digits for std, 8 for ext)
    if (msg.flags.extended)
        Serial.printf("%08X", msg.id);  // E.g. 1ABCDE1F
    else
        Serial.printf("%03X", msg.id);  // E.g. 123

    // 3. Print length (1 hex digit): always between 0 and 8
    Serial.printf("%1X", msg.len);  // E.g. 8

    // 4. Print each byte in 2-digit hex (pad with 0 if needed)
    for (uint8_t i = 0; i < msg.len; i++)
        Serial.printf("%02X", msg.buf[i]);  // E.g. DE

    // 5. End of frame per SLCAN spec — carriage return only
    Serial.print('\r');  // Important: NOT println()
}
```