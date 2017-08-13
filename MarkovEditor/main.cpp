//
//  main.cpp
//  MarkovEditor
//
//  Created by Jonathan B Gallagher on 6/30/17.
//  Copyright © 2017 Jonathan B Gallagher. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <ctype.h>
#include <errno.h>

#ifdef __MWERKS__
# define FL_DLL
#endif

#include <FL/Fl.H>
#include <FL/x.H> // for fl_open_callback
#include <FL/Fl_ask.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <Fl/Fl_Value_Slider.H>
#include <Fl/Fl_Choice.H>
#include <FL/filename.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include "MarkovWordChain.hpp"


int                changed = 0;
char               filename[FL_PATH_MAX] = "";
char               title[FL_PATH_MAX];
Fl_Text_Buffer     *textbuf = 0;
map<int,Fl_Label *>wordChoiceLables;
map<int,Fl_Button *>wordChoiceChecks;
vector<Fl_Input *> wordChoiceInputs;
Fl_Double_Window *wordChoices;
Fl_Box *bxlabel;
Fl_Scroll *wordChoiceScroll;
Fl_Output *baseWord;
Fl_Output *baseCount;
Fl_Output *nextWord;
Fl_Output *nextCount;
Fl_Output *nextPerc;
Fl_Button *wcOk;

int maxWidth = 800;
int maxHeight = 600;
int nWordsPerLine = 0;
int nLinesPerParagraph = 0;
int maxWordsPerLine;
int maxLinesPerParagraph;
int firstLinePos = 0;
bool newGen = true;
MarkovWordChain *myMarkovChain = new MarkovWordChain();

// width of line number display, if enabled
const int line_num_width = 75;

// #define DEV_TEST		// uncomment this line ...
// ... to enable additional test features for developers,
// particularly to test Fl_Text_Display and/or Fl_Text_Editor.

// Syntax highlighting stuff...
#define TS 24 // default editor textsize
// Editor window functions and class...
void save_cb();
void saveas_cb();
void find2_cb(Fl_Widget*, void*);
void replall_cb(Fl_Widget*, void*);
void replace2_cb(Fl_Widget*, void*);
void replcan_cb(Fl_Widget*, void*);

class EditorWindow : public Fl_Double_Window {
public:
    EditorWindow(int w, int h, const char* t);
    ~EditorWindow();
    
    Fl_Window          *replace_dlg;
    Fl_Input           *replace_find;
    Fl_Input           *replace_with;
    Fl_Button          *replace_all;
    Fl_Return_Button   *replace_next;
    Fl_Button          *replace_cancel;
    

    
    Fl_Button *genText;
    Fl_Label *sLabel;
    Fl_Label *nLabel;
    Fl_Label *lLabel;
    Fl_Input *seedWord;
    Fl_Input *numberOfWords;
    Fl_Value_Slider *nLevelSlider;
    Fl_Value_Slider *wordsPerLine;
    Fl_Value_Slider *linesPerParagraph;
    Fl_Choice *markovMode;
    Fl_Choice *markovDist;
    
    Fl_Check_Button *aprxWords;
    Fl_Check_Button *aprxLines;
    
    Fl_Value_Slider *parBegin;
    Fl_Value_Slider *parEnd;
    Fl_Value_Slider *seedValue;
    Fl_Value_Slider *keepItr;
    
    
    int			wrap_mode;
    int			line_numbers;
    
    Fl_Text_Editor     *editor;
    char               search[256];
};

EditorWindow::EditorWindow(int w, int h, const char* t) : Fl_Double_Window(w, h, t) {
    replace_dlg = new Fl_Window(300, 105, "Replace");
    replace_find = new Fl_Input(80, 10, 210, 25, "Find:");
    replace_find->align(FL_ALIGN_LEFT);
    
    replace_with = new Fl_Input(80, 40, 210, 25, "Replace:");
    replace_with->align(FL_ALIGN_LEFT);
    
    replace_all = new Fl_Button(10, 70, 90, 25, "Replace All");
    replace_all->callback((Fl_Callback *)replall_cb, this);
    
    replace_next = new Fl_Return_Button(105, 70, 120, 25, "Replace Next");
    replace_next->callback((Fl_Callback *)replace2_cb, this);
    
    replace_cancel = new Fl_Button(230, 70, 60, 25, "Cancel");
    replace_cancel->callback((Fl_Callback *)replcan_cb, this);
    replace_dlg->end();
    replace_dlg->set_non_modal();
    editor = 0;
    *search = (char)0;
    wrap_mode = 0;
    line_numbers = 0;
}

EditorWindow::~EditorWindow() {
    delete replace_dlg;
}


int check_save(void) {
    if (!changed) return 1;
    
    int r = fl_choice("The current file has not been saved.\n"
                      "Would you like to save it now?",
                      "Cancel", "Save", "Don't Save");
    
    if (r == 1) {
        save_cb(); // Save the file...
        return !changed;
    }
    
    return (r == 2) ? 1 : 0;
}

int loading = 0;
void load_file(const char *newfile, int ipos) {
    loading = 1;
    int insert = (ipos != -1);
    changed = insert;
    if (!insert) strcpy(filename, "");
    int r;
    if (!insert) r = textbuf->loadfile(newfile);
    else r = textbuf->insertfile(newfile, ipos);
    changed = changed || textbuf->input_file_was_transcoded;
    if (r)
        fl_alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
    else
        if (!insert) strcpy(filename, newfile);
    loading = 0;
    textbuf->call_modify_callbacks();
}

void save_file(const char *newfile) {
    if (textbuf->savefile(newfile))
        fl_alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
    else
        strcpy(filename, newfile);
    changed = 0;
    textbuf->call_modify_callbacks();
}

void copy_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Text_Editor::kf_copy(0, e->editor);
}

void cut_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Text_Editor::kf_cut(0, e->editor);
}

void delete_cb(Fl_Widget*, void*) {
    textbuf->remove_selection();
}

void linenumbers_cb(Fl_Widget *w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Menu_Bar* m = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* i = m->mvalue();
    if ( i->value() ) {
        e->editor->linenumber_width(line_num_width);	// enable
        e->editor->linenumber_size(e->editor->textsize());
    } else {
        e->editor->linenumber_width(0);	// disable
    }
    e->line_numbers = (i->value()?1:0);
    e->redraw();
}

void wordwrap_cb(Fl_Widget *w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Menu_Bar* m = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* i = m->mvalue();
    if ( i->value() )
        e->editor->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    else
        e->editor->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);
    e->wrap_mode = (i->value()?1:0);
    e->redraw();
}

void find_cb(Fl_Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char *val;
    
    val = fl_input("Search String:", e->search);
    if (val != NULL) {
        // User entered a string - go find it!
        strcpy(e->search, val);
        find2_cb(w, v);
    }
}

void find2_cb(Fl_Widget* w, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    if (e->search[0] == '\0') {
        // Search string is blank; get a new one...
        find_cb(w, v);
        return;
    }
    
    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, e->search, &pos);
    if (found) {
        // Found a match; select and update the position...
        textbuf->select(pos, pos+strlen(e->search));
        e->editor->insert_position(pos+strlen(e->search));
        e->editor->show_insert_position();
    }
    else fl_alert("No occurrences of \'%s\' found!", e->search);
}

void set_title(Fl_Window* w) {
    if (filename[0] == '\0') strcpy(title, "Untitled");
    else {
        char *slash;
        slash = strrchr(filename, '/');
#ifdef WIN32
        if (slash == NULL) slash = strrchr(filename, '\\');
#endif
        if (slash != NULL) strcpy(title, slash + 1);
        else strcpy(title, filename);
    }
    
    if (changed) strcat(title, " (modified)");
    
    w->label(title);
}

void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) {
    if ((nInserted || nDeleted) && !loading) changed = 1;
    EditorWindow *w = (EditorWindow *)v;
    set_title(w);
    if (loading) w->editor->show_insert_position();
}

void new_cb(Fl_Widget*, void*) {
    if (!check_save()) return;
    
    filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    changed = 0;
    textbuf->call_modify_callbacks();
}

void open_cb(Fl_Widget*, void*) {
    if (!check_save()) return;
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Open file");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if ( fnfc.show() ) return;
    load_file(fnfc.filename(), -1);
    
}

void insert_cb(Fl_Widget*, void *v) {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Insert file");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if ( fnfc.show() ) return;
    EditorWindow *w = (EditorWindow *)v;
    load_file(fnfc.filename(), w->editor->insert_position());
}

void paste_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    Fl_Text_Editor::kf_paste(0, e->editor);
}

int num_windows = 0;

void close_cb(Fl_Widget*, void* v) {
    EditorWindow* w = (EditorWindow*)v;
    
    if (num_windows == 1) {
        if (!check_save())
            return;
    }
    
    w->hide();
    w->editor->buffer(0);
    textbuf->remove_modify_callback(changed_cb, w);
    Fl::delete_widget(w);
    
    num_windows--;
    if (!num_windows) exit(0);
}

void quit_cb(Fl_Widget*, void*) {
    if (changed && !check_save())
        return;
    
    exit(0);
}

void replace_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->show();
}

void replace2_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char *find = e->replace_find->value();
    const char *replace = e->replace_with->value();
    
    if (find[0] == '\0') {
        // Search string is blank; get a new one...
        e->replace_dlg->show();
        return;
    }
    
    e->replace_dlg->hide();
    
    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, find, &pos);
    
    if (found) {
        // Found a match; update the position and replace text...
        textbuf->select(pos, pos+strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
        textbuf->select(pos, pos+strlen(replace));
        e->editor->insert_position(pos+strlen(replace));
        e->editor->show_insert_position();
    }
    else fl_alert("No occurrences of \'%s\' found!", find);
}

void replall_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    const char *find = e->replace_find->value();
    const char *replace = e->replace_with->value();
    
    find = e->replace_find->value();
    if (find[0] == '\0') {
        // Search string is blank; get a new one...
        e->replace_dlg->show();
        return;
    }
    
    e->replace_dlg->hide();
    
    e->editor->insert_position(0);
    int times = 0;
    
    // Loop through the whole string
    for (int found = 1; found;) {
        int pos = e->editor->insert_position();
        found = textbuf->search_forward(pos, find, &pos);
        
        if (found) {
            // Found a match; update the position and replace text...
            textbuf->select(pos, pos+strlen(find));
            textbuf->remove_selection();
            textbuf->insert(pos, replace);
            e->editor->insert_position(pos+strlen(replace));
            e->editor->show_insert_position();
            times++;
        }
    }
    
    if (times) fl_message("Replaced %d occurrences.", times);
    else fl_alert("No occurrences of \'%s\' found!", find);
}

void replcan_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->replace_dlg->hide();
}

void save_cb() {
    if (filename[0] == '\0') {
        // No filename - get one!
        saveas_cb();
        return;
    }
    else save_file(filename);
}

void saveas_cb() {
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Save File As?");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    if ( fnfc.show() ) return;
    save_file(fnfc.filename());
}

Fl_Window* new_view();

void view_cb(Fl_Widget*, void*) {
    Fl_Window* w = new_view();
    w->show();
}


void load_MarkovData(const char *newfile)
{
    string fname(newfile);
    myMarkovChain->LoadTextIntoVector(fname);
    //myMarkovChain->LoadTextFromWordsToLetters();
}

void loadMarkov_cb(Fl_Widget*, void*v)
{
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Load Text for Markov Chain");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if ( fnfc.show() ) return;
    load_MarkovData(fnfc.filename());
    
}

void gen_SetChaosMap(void *v)
{
    EditorWindow* e = (EditorWindow*)v;
    myMarkovChain->setUseChaosMap(true);
    myMarkovChain->setParBegin(e->parBegin->value());
    myMarkovChain->setParEnd(e->parEnd->value());
    myMarkovChain->setItr(1000);
    myMarkovChain->setMpar(4.9);
    myMarkovChain->setX(e->seedValue->value());
    myMarkovChain->setKit(myMarkovChain->getItr()-(int)e->keepItr->value());
    myMarkovChain->setSteps(atoi(e->numberOfWords->value()));
    printf("CM: %f %f %f\n",myMarkovChain->getParBegin(),myMarkovChain->getParEnd(),myMarkovChain->getX());
}

void gen_SetupDistrib(void *v)
{
    EditorWindow* e = (EditorWindow*)v;
    switch (e->markovDist->value()) {
        case 0:
            myMarkovChain->setUseChaosMap(false);
            break;
        case 1:
            gen_SetChaosMap(v);
            myMarkovChain->setMyMapType(LOG);
            break;
        case 2:
            gen_SetChaosMap(v);
            myMarkovChain->setMyMapType(EXP);
            break;
        case 3:
            gen_SetChaosMap(v);
            myMarkovChain->setMyMapType(MOUSE);
            break;
        default:
            break;
    }
}

void gen_SetUpMarkovChain(void *v)
{
    EditorWindow* e = (EditorWindow*)v;
    newGen = true;
    gen_SetupDistrib(v);
    if(e->markovMode->value() == 0) {
        myMarkovChain->SetupCharMarkovChain();
        myMarkovChain->setMarkovMode(BY_CHAR);
    }
    if(e->markovMode->value() == 1) {
        myMarkovChain->SetupMarkovChain();
        myMarkovChain->setMarkovMode(BY_WORD);
    }
    if(e->markovMode->value() == 2) {
        myMarkovChain->SetupNLevelMarkovChain();
        myMarkovChain->setMarkovMode(BY_NWORD);
    }
}

void gen_ClearMarkovChain()
{
    switch (myMarkovChain->getMarkovMode()) {
        case BY_CHAR:
            myMarkovChain->ClearMarkovChain();
            break;
        case BY_WORD:
            myMarkovChain->ClearMarkovChain();
            break;
        case BY_NWORD:
            myMarkovChain->ClearNLevelMarkovChain();
            break;
            
        default:
            break;
    }
}

bool gen_GetNextMarkovChain()
{
    bool fw;
    switch (myMarkovChain->getMarkovMode()) {
        case BY_CHAR:
            if(myMarkovChain->GetNextProbChainByChar()) {
                myMarkovChain->GetNextStrInProbChainByChar();
                fw = true;
            } else {
                fw = false;
            }
            break;
        case BY_WORD:
            if(myMarkovChain->GetNextProbChain()) {
                myMarkovChain->GetNextWordInProbChain();
                fw = true;
            } else {
                fw = false;
            }
            break;
        case BY_NWORD:
            if(myMarkovChain->GetNextNLevelProbChain()) {
                myMarkovChain->GetNextWordInNLevelProbChain();
                fw = true;
            } else {
                fw = false;
            }
            break;
        default:
            break;
    }
    return fw;
}

void gen_SetMaxNumWords(void *v)
{
    EditorWindow* e = (EditorWindow*)v;
    maxWordsPerLine = (e->aprxWords->value() == 1) ? e->wordsPerLine->value() + floor((float)rand()/RAND_MAX*e->wordsPerLine->value()) :
    e->wordsPerLine->value();
}

void gen_SetMaxNumLines(void *v)
{
    EditorWindow* e = (EditorWindow*)v;
    maxLinesPerParagraph = (e->aprxLines->value() == 1) ? e->linesPerParagraph->value() + floor((float)rand()/RAND_MAX*e->linesPerParagraph->value()) :
    e->linesPerParagraph->value();

}


void gen_ProcessTextByChar(int num, void *v)
{
    string lineString;
    int fpos = -1;
    int ppos = 0;
    int found = -1;
    int back_found = -1;
    int bpos = -1;
    int pbos = 0;
    //adjust size of number of words based on number of iterates kept from each step in a chaotic map
    //(Markov Distributions: Logistic Map, Exponential Map, Mouse Map), the number of words is the number
    //of steps in the calculation and "KeepItr" is the number of numbers kept for each step
    if(myMarkovChain->getUseChaosMap())
        num = num * (myMarkovChain->getItr()-myMarkovChain->getKit());
    for(int i=0; i<num; i++) {
        if(i == 0)
            textbuf->append(myMarkovChain->getSeedWord().c_str());
        if(gen_GetNextMarkovChain()) {
            //char *fs = &myMarkovChain->GetFoundWord().at(myMarkovChain->GetFoundWord().size()-1);
            textbuf->append(myMarkovChain->GetFoundWord().c_str());
            found = textbuf->search_forward(ppos, " ", &fpos);
            ppos = fpos+1;
            while (found) {
                printf("Found space at %d %d\n",fpos,ppos);
                found = textbuf->search_forward(ppos, " ", &fpos);
                ppos = fpos+1;
                nWordsPerLine++;
            }
            //nWordsPerLine++;
            //for by char mode try to see if we are ending on a space or punctuation, so that
            //we end on a word or end of sentence (may need to look into further)
            if(nWordsPerLine >= maxWordsPerLine) {
                    textbuf->replace(ppos-1, ppos, "\n");
                    //textbuf->append("\n");
                    nWordsPerLine = 0;
                    gen_SetMaxNumWords(v);
                    nLinesPerParagraph++;
            }
                if(nLinesPerParagraph == maxLinesPerParagraph) {
                    back_found = textbuf->search_backward(textbuf->length()-1, "\n", &bpos);
                    textbuf->insert(bpos+1, "\n");
                    //textbuf->append("\n");
                    //textbuf->append("\n");
                    gen_SetMaxNumWords(v);
                    gen_SetMaxNumLines(v);
                    nLinesPerParagraph = 0;
                }
                /*if(nLinesPerParagraph == maxLinesPerParagraph) {
                    textbuf->append("\n");
                    textbuf->append("\n");
                    //textbuf->append("\t");
                    gen_SetMaxNumWords(v);
                    gen_SetMaxNumLines(v);
                    nLinesPerParagraph = 0;
                    nWordsPerLine = 0;
                }*/

           // }
        } else {
            fl_alert("String: %s not in Markov Database!",myMarkovChain->GetFirstWord().c_str());
            i = num;
            myMarkovChain->ClearMarkovChain();
        }
    }
}

void gen_ProcessTextByWord(int num, void *v)
{
    if(myMarkovChain->getUseChaosMap())
        num = num * (myMarkovChain->getItr()-myMarkovChain->getKit());

    for(int i=0; i<num; i++) {
        if(i == 0) {
            textbuf->append(myMarkovChain->getSeedWord().c_str());
            textbuf->append(" ");
        }
        if(gen_GetNextMarkovChain()) {
            //append word and a space to the text buffer
            textbuf->append(myMarkovChain->GetFoundWord().c_str());
            textbuf->append(" ");
            nWordsPerLine++;
            //for by char mode try to see if we are ending on a space or punctuation, so that
            //we end on a word or end of sentence (may need to look into further)
            if(nWordsPerLine == maxWordsPerLine) {
                textbuf->append("\n");
                nWordsPerLine = 0;
                gen_SetMaxNumWords(v);
                nLinesPerParagraph++;
            }
            if(nLinesPerParagraph == maxLinesPerParagraph) {
                textbuf->append("\n");
                textbuf->append("\n");
                //textbuf->append("\t");
                gen_SetMaxNumWords(v);
                gen_SetMaxNumLines(v);
                nLinesPerParagraph = 0;
                nWordsPerLine = 0;
            }
        } else {
            fl_alert("Word: %s not it Markov Database!",myMarkovChain->GetFirstWord().c_str());
            i = num;
            if(myMarkovChain->getMarkovMode() == BY_WORD)
                myMarkovChain->ClearMarkovChain();
            if(myMarkovChain->getMarkovMode() == BY_NWORD)
                myMarkovChain->ClearNLevelMarkovChain();
        }
    }
}

void gen_GenerateText(int num, void* v)
{
    switch (myMarkovChain->getMarkovMode()) {
        case BY_CHAR:
            gen_ProcessTextByChar(num,v);
            break;
        case BY_NWORD:
            gen_ProcessTextByWord(num,v);
            break;
        case BY_WORD:
            gen_ProcessTextByWord(num,v);
            break;
        default:
            break;
    }
}

bool gen_setSeedWord(void *v)
{
    bool setSeedWord;
    string str;
    EditorWindow* e = (EditorWindow*)v;
    if(e->markovMode->value() == 0) {
        
        str.append(e->seedWord->value());
        
        if(str.size() >= e->nLevelSlider->value()) {
            if(str.size() == e->nLevelSlider->value()) {
                myMarkovChain->setSeedWord(e->seedWord->value());
            } else {
                string str2;
                str2 = str.substr(((int)str.size()-e->nLevelSlider->value()),e->nLevelSlider->value());
                myMarkovChain->setSeedWord(str2);
            }
            setSeedWord = true;
        } else {
            fl_alert("BY_CHAR: SeedWord: %s NumLevels: %d \n Number of Levels must be less than or equal to number of characters in seed word!",myMarkovChain->getSeedWord().c_str(),(int)e->nLevelSlider->value());
            setSeedWord = false;
        }
    } else {
        myMarkovChain->setSeedWord(e->seedWord->value());
        setSeedWord = true;
    }
    return setSeedWord;
}

void genText_cb(Fl_Widget* w, void* v)
{
    EditorWindow* e = (EditorWindow*)v;
    int num = atoi(e->numberOfWords->value());
    printf("Num words is: %d %s\n",num,e->seedWord->value());
    myMarkovChain->setSeedWord(e->seedWord->value());
    if(gen_setSeedWord(v)) {
        myMarkovChain->setNumLevels(e->nLevelSlider->value());
        if(!myMarkovChain->getChainIsReady()) {
            gen_SetUpMarkovChain(v);
            gen_SetMaxNumWords(v);
            gen_SetMaxNumLines(v);
        } else {
            textbuf->text("");
            gen_ClearMarkovChain();
            gen_SetUpMarkovChain(v);
            nWordsPerLine = 0;
            nLinesPerParagraph = 0;
            gen_SetMaxNumWords(v);
            gen_SetMaxNumLines(v);
        }
        gen_GenerateText(num, v);
    }
}

void deleteWordChoiceObjetcs()
{
    for(map<int,Fl_Button *>::iterator it = wordChoiceChecks.begin(); it != wordChoiceChecks.end(); it++) {
        delete it->second;
    }
    delete nextPerc;
    delete nextCount;
    delete nextWord;
    delete baseCount;
    delete baseWord;
    delete wcOk;
    delete wordChoiceScroll;
    delete wordChoices;

    wordChoiceChecks.clear();
}

void wcOk_cb()
{
    
    int nextStart,nextEnd = -1;
    textbuf->secondary_selection_position(&nextStart, &nextEnd);
    textbuf->replace(nextStart, nextEnd, "");
    textbuf->insert(nextStart, nextWord->value());
    deleteWordChoiceObjetcs();
    
}

void wcRadio_cb()
{
    for(map<int,Fl_Button *>::iterator it = wordChoiceChecks.begin(); it != wordChoiceChecks.end(); it++) {
        if(it->second->value() == 1) {
            string selWord = textbuf->selection_text();
            nextWord->value(it->second->label());
            char dnum[6];
            sprintf(dnum, "%d", (int)myMarkovChain->GetNextWordCount(selWord, it->second->label()));
            nextCount->value(dnum);
            sprintf(dnum, "%0.3f", myMarkovChain->GetNextWordPerc(selWord, it->second->label()));
            nextPerc->value(dnum);
            
        }
    }
}


void wordChoices_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    int rows,cols,leftover=0;
    int nWidth,nHeight,wcWidth,wcHeight = 0;
    int nextStart,nextEnd = -1;
    int baseStart,baseEnd = -1;
    int sel = -1;
    string selWord = textbuf->selection_text();
    sel = textbuf->selection_position(&baseStart, &baseEnd);
    
    bool notFoundNext = true;
    int fcnt = 0;
    nextStart = baseEnd+1;
    while (notFoundNext) {
        textbuf->search_forward(nextStart, " ", &nextEnd);
        fcnt++;
        if(fcnt == 2)
            notFoundNext = false;
        
    }
    textbuf->secondary_select(nextStart, nextEnd);
    textbuf->highlight(nextStart, nextEnd);
    string nextText = textbuf->secondary_selection_text();
    myMarkovChain->SetWordMapPointer(selWord);
   // Fl_Choice *words = new Fl_Choice(150, 50, 120, 30, "Word Choices");
    printf("MAP SIZE: %lu %s\n",myMarkovChain->wMapPointer->size(),selWord.c_str());
    
    cols = (int)ceil(sqrt((double)myMarkovChain->wMapPointer->size()));
    if(cols*cols < myMarkovChain->wMapPointer->size()) {
        rows = cols+1;
        leftover = (rows*cols) - (int)myMarkovChain->wMapPointer->size();
    } else {
        rows = cols;
    }
    printf("ROWS-COLS %d %d %d\n", rows,cols,leftover);
    nWidth = 100+(cols*130);
    nHeight = 100+(rows*40);
    if(nWidth > maxWidth || nHeight > maxHeight) {
        wcWidth = 800;
        wcHeight = 600;
    } else {
        wcWidth = nWidth;
        wcHeight = nHeight;
    }
    
    wordChoices = new Fl_Double_Window(wcWidth, wcHeight, "WordChoices");
    wordChoiceScroll = new Fl_Scroll(0,0,wcWidth,wcHeight);
    wordChoiceScroll->box(FL_DOWN_FRAME);
    
    baseWord = new Fl_Output(10,30,120,30,"Base Word:");
    baseWord->labelsize(10);
    baseWord->align(FL_ALIGN_TOP);
    baseWord->labelfont(FL_BOLD);
    baseWord->value(selWord.c_str());
    
    baseCount = new Fl_Output(130,30,60,30,"Count:");
    baseCount->labelsize(10);
    baseCount->align(FL_ALIGN_TOP);
    baseCount->labelfont(FL_BOLD);
    char dnum[6];
    sprintf(dnum, "%d",(int)myMarkovChain->GetBaseWordSum(selWord));
    baseCount->value(dnum);
    
    nextWord = new Fl_Output(190,30,120,30,"Next Word:");
    nextWord->labelsize(10);
    nextWord->align(FL_ALIGN_TOP);
    nextWord->labelfont(FL_BOLD);
    nextWord->value(nextText.c_str());
    
    nextCount = new Fl_Output(310,30,60,30,"Count:");
    nextCount->labelsize(10);
    nextCount->align(FL_ALIGN_TOP);
    nextCount->labelfont(FL_BOLD);
    sprintf(dnum, "%d",(int)myMarkovChain->GetNextWordCount(selWord, nextText));
    nextCount->value(dnum);
    
    nextPerc = new Fl_Output(370,30,60,30,"Percent:");
    nextPerc->labelsize(10);
    nextPerc->align(FL_ALIGN_TOP);
    nextPerc->labelfont(FL_BOLD);
    sprintf(dnum, "%0.3f",myMarkovChain->GetNextWordCount(selWord, nextText));
    nextPerc->value(dnum);


    
    wordMap::iterator itM = myMarkovChain->wMapPointer->begin();
    int c=0;
    for(int i=0; i<cols; i++) {
        for(int j=0; j<rows; j++) {
            if(c < (int)myMarkovChain->wMapPointer->size()) {
                Fl_Round_Button *wccb = new Fl_Round_Button(150+(j*130),75+(i*40), 30, 30, itM->first.c_str());
                wccb->type(FL_RADIO_BUTTON);
                wccb->labelsize(16);
                wccb->align(FL_ALIGN_LEFT);
                wccb->labelfont(FL_BOLD);
                wccb->callback((Fl_Callback *)wcRadio_cb, e);
                itM++;
                wordChoiceChecks[c] = wccb;
                c++;
            }
        }
    }

    
    wcOk = new Fl_Return_Button(20, 50+(rows*40), 120, 30, "&Replace Word");
    wcOk->labelsize(10);
    wcOk->labelfont(FL_BOLD);
    wcOk->callback((Fl_Callback *)wcOk_cb, e);
    
   

    wordChoices->end();
    wordChoices->show();

}

Fl_Menu_Item menuitems[] = {
    { "&File",              0, 0, 0, FL_SUBMENU },
    { "&New File",        0, (Fl_Callback *)new_cb },
    { "&Open File...",    FL_COMMAND + 'o', (Fl_Callback *)open_cb },
    { "&Insert File...",  FL_COMMAND + 'i', (Fl_Callback *)insert_cb},
    { "&Load Markov Text",  FL_COMMAND + 'm', (Fl_Callback *)loadMarkov_cb, 0, FL_MENU_DIVIDER },
    { "&Save File",       FL_COMMAND + 's', (Fl_Callback *)save_cb },
    { "Save File &As...", FL_COMMAND + FL_SHIFT + 's', (Fl_Callback *)saveas_cb, 0, FL_MENU_DIVIDER },
    { "New &View",        FL_ALT
#ifdef __APPLE__
        + FL_COMMAND
#endif
        + 'v', (Fl_Callback *)view_cb, 0 },
    { "&Close View",      FL_COMMAND + 'w', (Fl_Callback *)close_cb, 0, FL_MENU_DIVIDER },
    { "E&xit",            FL_COMMAND + 'q', (Fl_Callback *)quit_cb, 0 },
    { 0 },
    
    { "&Edit", 0, 0, 0, FL_SUBMENU },
    { "Cu&t",             FL_COMMAND + 'x', (Fl_Callback *)cut_cb },
    { "&Copy",            FL_COMMAND + 'c', (Fl_Callback *)copy_cb },
    { "&Paste",           FL_COMMAND + 'v', (Fl_Callback *)paste_cb },
    { "&Delete",          0, (Fl_Callback *)delete_cb },
    { "Preferences",      0, 0, 0, FL_SUBMENU },
    { "Line Numbers",   FL_COMMAND + 'l', (Fl_Callback *)linenumbers_cb, 0, FL_MENU_TOGGLE },
    { "Word Wrap",      0,                (Fl_Callback *)wordwrap_cb, 0, FL_MENU_TOGGLE },
    { 0 },
    { 0 },
    
    { "&Search", 0, 0, 0, FL_SUBMENU },
    { "&Find...",         FL_COMMAND + 'f', (Fl_Callback *)find_cb },
    { "F&ind Again",      FL_COMMAND + 'g', find2_cb },
    { "&Replace...",      FL_COMMAND + 'r', replace_cb },
    { "Re&place Again",   FL_COMMAND + 't', replace2_cb },
    { 0 },
    
    { "&WordTree", 0, 0, 0, FL_SUBMENU },
    { "&Show Word Choices", FL_COMMAND + 'k', (Fl_Callback *)wordChoices_cb},
    { 0 },
    
    { 0 }
};

void distribution_cb(Fl_Widget *w, void *v)
{
    EditorWindow *e = (EditorWindow *)v;
    if(e->markovDist->value() == 0) {
        e->parBegin->hide();
        e->parEnd->hide();
        e->keepItr->hide();
        e->seedValue->hide();
    }
    if(e->markovDist->value() == 1) {
        e->parBegin->bounds(2.0, 4.0);
        e->parBegin->step(0.1);
        e->parBegin->value(2.8);
        e->parBegin->show();
        e->parEnd->bounds(2.0, 4.0);
        e->parEnd->step(0.1);
        e->parEnd->value(4.0);
        e->parEnd->show();
        e->seedValue->value(0.1);
        e->seedValue->show();
        e->keepItr->value(1);
        e->keepItr->show();
    }
    if(e->markovDist->value() == 2) {
        e->parBegin->bounds(0.57, 0.135);
        e->parBegin->step(-0.05);
        e->parBegin->value(0.57);
        e->parBegin->show();
        e->parEnd->bounds(0.57, 0.135);
        e->parEnd->step(-0.05);
        e->parEnd->value(0.135);
        e->parEnd->show();
        e->seedValue->value(0.1);
        e->seedValue->show();
        e->keepItr->value(1);
        e->keepItr->show();
    }
    if(e->markovDist->value() == 3) {
        e->parBegin->bounds(-1.0, 1.0);
        e->parBegin->step(0.1);
        e->parBegin->value(-0.75);
        e->parBegin->show();
        e->parEnd->bounds(-1.0, 1.0);
        e->parEnd->step(0.1);
        e->parEnd->value(0.75);
        e->parEnd->show();
        e->seedValue->value(0.1);
        e->seedValue->show();
        e->keepItr->value(1);
        e->keepItr->show();
    }
}

Fl_Window* new_view() {
    EditorWindow* w = new EditorWindow(1024, 768, title);
    
    w->begin();
    Fl_Menu_Bar* m = new Fl_Menu_Bar(0, 0, 1024, 30);
    m->copy(menuitems, w);
    w->editor = new Fl_Text_Editor(0, 30, 1024, 600);
    w->editor->textfont(FL_COURIER);
    w->editor->textsize(TS);
    w->editor->wrap_mode(Fl_Text_Editor::WRAP_AT_BOUNDS, 250);
    w->editor->buffer(textbuf);
    

    
    w->genText = new Fl_Button(20, 650, 120, 30, "&Generate Text");
    w->genText->labelsize(10);
    w->genText->labelfont(FL_BOLD);
    w->genText->callback(genText_cb,w);
    //level slider
    w->nLevelSlider = new Fl_Value_Slider(20,700, 120, 30, "Number of Levels");
    w->nLevelSlider->type(FL_HOR_NICE_SLIDER);
    w->nLevelSlider->step(1);
    w->nLevelSlider->value(1);
    w->nLevelSlider->labelsize(10);
    w->nLevelSlider->labelfont(FL_BOLD);
    w->nLevelSlider->align(FL_ALIGN_TOP);
    w->nLevelSlider->bounds(1, 25);
    
    //seed word
    w->seedWord = new Fl_Input(160,650,120,30, "&Seed Word");
    w->seedWord->align(FL_ALIGN_TOP);
    w->seedWord->labelsize(10);
    w->seedWord->labelfont(FL_BOLD);
    //num words
    w->numberOfWords = new Fl_Input(160,700,120, 30, "&Number of Words");
    w->numberOfWords->labelsize(10);
    w->numberOfWords->align(FL_ALIGN_TOP);
    w->numberOfWords->labelfont(FL_BOLD);
    w->numberOfWords->value("1000");
    
    //markov mode
    w->markovMode = new Fl_Choice(300, 650, 120, 30, "&Markov Mode");
    w->markovMode->labelsize(10);
    w->markovMode->align(FL_ALIGN_TOP);
    w->markovMode->labelfont(FL_BOLD);
    w->markovMode->add("BY_CHAR");
    w->markovMode->add("BY_WORD");
    w->markovMode->add("BY_NWORD");
    w->markovMode->value(0);
    
    w->markovDist = new Fl_Choice(300,700,120, 30, "&Distribution");
    w->markovDist->labelsize(10);
    w->markovDist->align(FL_ALIGN_TOP);
    w->markovDist->labelfont(FL_BOLD);
    w->markovDist->add("RANDOM");
    w->markovDist->add("LOGISTIC MAP");
    w->markovDist->add("EXPONENTIAL MAP");
    w->markovDist->add("MOUSE MAP");
    w->markovDist->callback((Fl_Callback *)distribution_cb, w);
    w->markovDist->value(0);
    
    //randomize word/line counts
    w->aprxWords = new Fl_Check_Button(440,650,15, 30, "Rnd");
    w->aprxWords->type(FL_TOGGLE_BUTTON);
    w->aprxWords->labelsize(10);
    w->aprxWords->align(FL_ALIGN_TOP);
    w->aprxWords->labelfont(FL_BOLD);
    
    
    w->aprxLines = new Fl_Check_Button(440,700,15, 30, "Rnd");
    w->aprxLines->type(FL_TOGGLE_BUTTON);
    w->aprxLines->labelsize(10);
    w->aprxLines->align(FL_ALIGN_TOP);
    w->aprxLines->labelfont(FL_BOLD);

    
    //words per line slider
    w->wordsPerLine = new Fl_Value_Slider(475,650,120, 30, "Words Per Line");
    w->wordsPerLine->type(FL_HOR_NICE_SLIDER);
    w->wordsPerLine->step(1);
    w->wordsPerLine->value(15);
    w->wordsPerLine->bounds(1, 50);
    w->wordsPerLine->labelsize(10);
    w->wordsPerLine->labelfont(FL_BOLD);
    w->wordsPerLine->align(FL_ALIGN_TOP);
    
    w->linesPerParagraph = new Fl_Value_Slider(475,700,120, 30, "Lines Per Paragraph");
    w->linesPerParagraph->type(FL_HOR_NICE_SLIDER);
    w->linesPerParagraph->step(1);
    w->linesPerParagraph->value(15);
    w->linesPerParagraph->bounds(1, 250);
    w->linesPerParagraph->labelsize(10);
    w->linesPerParagraph->labelfont(FL_BOLD);
    w->linesPerParagraph->align(FL_ALIGN_TOP);

    //chaos map parameter sliders
    w->parBegin = new Fl_Value_Slider(615, 650, 120 , 30, "Par Begin");
    w->parBegin->type(FL_HOR_NICE_SLIDER);
    w->parBegin->labelsize(10);
    w->parBegin->labelfont(FL_BOLD);
    w->parBegin->align(FL_ALIGN_TOP);
    w->parBegin->hide();
    
    w->parEnd = new Fl_Value_Slider(615, 700, 120 , 30, "Par End");
    w->parEnd->type(FL_HOR_NICE_SLIDER);
    w->parEnd->labelsize(10);
    w->parEnd->labelfont(FL_BOLD);
    w->parEnd->align(FL_ALIGN_TOP);
    w->parEnd->hide();
    
    w->seedValue = new Fl_Value_Slider(755, 650, 120 ,30, "Seed Value");
    w->seedValue->type(FL_HOR_NICE_SLIDER);
    w->seedValue->step(0.025);
    w->seedValue->value(0.1);
    w->seedValue->bounds(0.0, 1.0);
    w->seedValue->labelsize(10);
    w->seedValue->labelfont(FL_BOLD);
    w->seedValue->align(FL_ALIGN_TOP);
    w->seedValue->hide();

    w->keepItr = new Fl_Value_Slider(755, 700, 120 ,30, "Keep Iter");
    w->keepItr->type(FL_HOR_NICE_SLIDER);
    w->keepItr->step(1);
    w->keepItr->value(1);
    w->keepItr->bounds(1, 50);
    w->keepItr->labelsize(10);
    w->keepItr->labelfont(FL_BOLD);
    w->keepItr->align(FL_ALIGN_TOP);
    w->keepItr->hide();


    w->end();
    w->resizable(w->editor);
    w->size_range(300,200);
    w->callback((Fl_Callback *)close_cb, w);
    
    textbuf->add_modify_callback(changed_cb, w);
    textbuf->call_modify_callbacks();
    num_windows++;
    return w;
}

void cb(const char *fname) {
    load_file(fname, -1);
}

int main(int argc, char **argv) {
    textbuf = new Fl_Text_Buffer;
    //textbuf->transcoding_warning_action = NULL;
    fl_open_callback(cb);
    
    Fl_Window* window = new_view();
    
    window->show(1, argv);
#ifndef __APPLE__
    if (argc > 1) load_file(argv[1], -1);
#endif
    
    return Fl::run();
}

//
// End of "$Id: editor.cxx 11204 2016-02-22 16:36:38Z AlbrechtS $".
//
