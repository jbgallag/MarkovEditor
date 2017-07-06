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
#include <ctype.h>
#include <errno.h>

#ifdef __MWERKS__
# define FL_DLL
#endif

#include <FL/Fl.H>
#include <FL/x.H> // for fl_open_callback
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <Fl/Fl_Value_Slider.H>
#include <FL/filename.H>
#include "MarkovWordChain.hpp"


int                changed = 0;
char               filename[FL_PATH_MAX] = "";
char               title[FL_PATH_MAX];
Fl_Text_Buffer     *textbuf = 0;
int nWordsPerLine = 0;
int nLinesPerParagraph = 0;
int maxWordsPerLine;
int maxLinesPerParagraph;
int firstLinePos = 0;
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
    
}

void loadMarkov_cb(Fl_Widget*, void*)
{
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Load Text for Markov Chain");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if ( fnfc.show() ) return;
    load_MarkovData(fnfc.filename());
    
}

void genText_cb(Fl_Widget* w, void* v)
{
    EditorWindow* e = (EditorWindow*)v;
    int num = atoi(e->numberOfWords->value());
    printf("Num words is: %d %s\n",num,e->seedWord->value());
    myMarkovChain->setSeedWord(e->seedWord->value());
    myMarkovChain->setNumLevels(e->nLevelSlider->value());
    if(!myMarkovChain->getChainIsReady()) {
        myMarkovChain->SetupMarkovChain();
        maxWordsPerLine = e->wordsPerLine->value() + floor((float)rand()/RAND_MAX*e->wordsPerLine->value());
        maxLinesPerParagraph = e->linesPerParagraph->value() + floor((float)rand()/RAND_MAX*e->linesPerParagraph->value());
        printf("MAX: %d %d %f\n",maxWordsPerLine,maxLinesPerParagraph,(float)rand()/RAND_MAX);
    } else {
        textbuf->text("");
        myMarkovChain->ClearMarkovChain();
        myMarkovChain->SetupMarkovChain();
        nWordsPerLine = 0;
        nLinesPerParagraph = 0;
        maxWordsPerLine = e->wordsPerLine->value() + floor((float)rand()/RAND_MAX*e->wordsPerLine->value());
        maxLinesPerParagraph = e->linesPerParagraph->value() + floor((float)rand()/RAND_MAX*e->linesPerParagraph->value());

    }
    for(int i=0; i<num; i++) {
        myMarkovChain->GetNextProbChain();
        myMarkovChain->GetNextWordInProbChain();
       // printf("%s\n",myMarkovChain->GetFoundWord().c_str());
        
            
        
        //make paragraphs only if ending on punctuation
       // if(nLinesPerParagraph  >= maxLinesPerParagraph) {
         //   textbuf->append("\n");
         //   textbuf->append("\t");
          //  maxLinesPerParagraph = e->linesPerParagraph->value() + floor((float)rand()/RAND_MAX*e->linesPerParagraph->value());
          //  nLinesPerParagraph = 0;
       // }

        if(ispunct(myMarkovChain->GetFoundWord().c_str()[0])) {
            textbuf->append(myMarkovChain->GetFoundWord().c_str());
            if(nWordsPerLine >= maxWordsPerLine) {
                textbuf->append("\n");
                nWordsPerLine = 0;
                maxWordsPerLine = e->wordsPerLine->value() + floor((float)rand()/RAND_MAX*e->wordsPerLine->value());
                nLinesPerParagraph++;
                
            }
            if(nLinesPerParagraph  >= maxLinesPerParagraph) {
                textbuf->append("\n");
                textbuf->append("\n");
                textbuf->append("\t");
                maxLinesPerParagraph = e->linesPerParagraph->value() + floor((float)rand()/RAND_MAX*e->linesPerParagraph->value());
                nLinesPerParagraph = 0;
                nWordsPerLine = 0;
            } else {
                nWordsPerLine++;
            }
        } else {
            textbuf->append(" ");
            textbuf->append(myMarkovChain->GetFoundWord().c_str());
            nWordsPerLine++;
        }
        
    }
    
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
    
    { 0 }
};

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
    
    //words per line slider
    w->wordsPerLine = new Fl_Value_Slider(300,650,120, 30, "Words Per Line");
    w->wordsPerLine->type(FL_HOR_NICE_SLIDER);
    w->wordsPerLine->step(1);
    w->wordsPerLine->value(15);
    w->wordsPerLine->bounds(1, 50);
    w->wordsPerLine->labelsize(10);
    w->wordsPerLine->labelfont(FL_BOLD);
    w->wordsPerLine->align(FL_ALIGN_TOP);
    
    w->linesPerParagraph = new Fl_Value_Slider(300,700,120, 30, "Lines Per Paragraph");
    w->linesPerParagraph->type(FL_HOR_NICE_SLIDER);
    w->linesPerParagraph->step(1);
    w->linesPerParagraph->value(15);
    w->linesPerParagraph->bounds(1, 250);
    w->linesPerParagraph->labelsize(10);
    w->linesPerParagraph->labelfont(FL_BOLD);
    w->linesPerParagraph->align(FL_ALIGN_TOP);

    
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
