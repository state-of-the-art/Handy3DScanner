#ifndef SCENEFILE_H
#define SCENEFILE_H

#include <QObject>

namespace Qt3DCore {
    class QEntity;
}

class SceneFile
    : public QObject
{
    Q_OBJECT

public:
    inline static SceneFile* I() { if( s_pInstance == nullptr ) s_pInstance = new SceneFile(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    Q_INVOKABLE bool saveScene(Qt3DCore::QEntity *scene, const QString &file_path);

private:
    static SceneFile *s_pInstance;
    explicit SceneFile();
    ~SceneFile() override;
};

#endif // SCENEFILE_H
