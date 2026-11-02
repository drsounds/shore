#pragma once
#include <string>

namespace shore {

class SHApp;
class SHDocument;
class SHView;
class SHFrame;
class SHFramePeer;
class SHFeature;
class SHConcept;
class SHFeatureRegistry;
class SHTool;
class SHService;
class SHDocumentObject;
class SHFileType;
class SHPanel;

struct SHColor { unsigned char r = 0, g = 0, b = 0; };
struct SHRect { int x = 0, y = 0, width = 0, height = 0; };

enum class SHRenderMode { Editing, Preview, Print, Export };
enum class SHDockPosition { Left, Right, Top, Bottom };
enum class SHControlType { Label, Button, TextField, CheckBox, Choice };

struct SHProperty {
    std::string name;
    std::string value;
};

struct SHControl {
    int id;
    SHControlType type;
    std::string label;
    std::string value;
};

}  // namespace shore
