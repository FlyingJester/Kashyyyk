#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Individual strings returned should be `free'ed.
 Parse parameters (const char **) should have a NULL element on the end.
*/

unsigned CSV_CountElements(const char *);
const char **CSV_ParseString(const char *);
const char *CSV_ConstructString(const char **);
void CSV_FreeParse(const char **);

#ifdef __cplusplus
}

#include <string>
#include <vector>

namespace FJ {

    namespace CSV {

        inline unsigned CountElements(const char *a){
            return CSV_CountElements(a);
        }

        template<typename T>
        unsigned CountElements(const T &a){
            return CSV_CountElements(a.c_str());
        }

        inline const char **ParseString(const char *a){
            return CSV_ParseString(a);
        }

        inline const char **ParseString(char *const a){
            return CSV_ParseString(a);
        }

        template<typename T>
        const char **ParseString(const T &a){
            return CSV_ParseString(a.c_str());
        }

        template<class T>
        const char *ConstructString(T &container){

            std::vector<typename T::T> vec(container.begin(), container.end());

            return CSV_ConstructString(&(vec.front()));
        }

        template<>
        inline const char *ConstructString<const char **>(const char ** &c_array){
            return CSV_ConstructString(c_array);
        }

        template<>
        inline const char *ConstructString<std::vector<const char *> >(std::vector<const char *> &container){
            return CSV_ConstructString(&(container.front()));
        }

        inline void FreeParse(const char **a){
            CSV_FreeParse(a);
        }

    }

}

#endif
