//
//  Spanner.h
//  libraries/metavoxels/src
//
//  Created by Andrzej Kapolka on 11/10/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Spanner_h
#define hifi_Spanner_h

#include <glm/glm.hpp>

#include "AttributeRegistry.h"
#include "MetavoxelUtil.h"

class HeightfieldColor;
class HeightfieldHeight;
class HeightfieldMaterial;
class SpannerRenderer;

/// An object that spans multiple octree cells.
class Spanner : public SharedObject {
    Q_OBJECT
    Q_PROPERTY(Box bounds MEMBER _bounds WRITE setBounds NOTIFY boundsChanged DESIGNABLE false)
    Q_PROPERTY(float placementGranularity MEMBER _placementGranularity DESIGNABLE false)
    Q_PROPERTY(float voxelizationGranularity MEMBER _voxelizationGranularity DESIGNABLE false)
    
public:
    
    /// Returns the value of the global visit counter and increments it.
    static int getAndIncrementNextVisit() { return _nextVisit.fetchAndAddOrdered(1); }
    
    Spanner();
    
    void setBounds(const Box& bounds);
    const Box& getBounds() const { return _bounds; }
    
    void setPlacementGranularity(float granularity) { _placementGranularity = granularity; }
    float getPlacementGranularity() const { return _placementGranularity; }
    
    void setVoxelizationGranularity(float granularity) { _voxelizationGranularity = granularity; }
    float getVoxelizationGranularity() const { return _voxelizationGranularity; }
    
    void setMerged(bool merged) { _merged = merged; }
    bool isMerged() const { return _merged; }
    
    /// Checks whether we've visited this object on the current traversal.  If we have, returns false.
    /// If we haven't, sets the last visit identifier and returns true.
    bool testAndSetVisited(int visit);

    /// Returns a pointer to the renderer, creating it if necessary.
    SpannerRenderer* getRenderer();

    /// Finds the intersection between the described ray and this spanner.
    virtual bool findRayIntersection(const glm::vec3& origin, const glm::vec3& direction, float& distance) const;

    /// Checks whether this spanner has its own colors.
    virtual bool hasOwnColors() const;

    /// Checks whether this spanner has its own materials.
    virtual bool hasOwnMaterials() const;

    /// Checks whether the spanner contains the specified point.
    virtual bool contains(const glm::vec3& point);

    /// Retrieves the color at the specified point.
    virtual QRgb getColorAt(const glm::vec3& point);
    
    /// Retrieves the material at the specified point.
    virtual int getMaterialAt(const glm::vec3& point);

    /// Retrieves a reference to the list of materials.
    virtual QVector<SharedObjectPointer>& getMaterials();

    /// Finds the intersection, if any, between the specified line segment and the spanner.
    virtual bool intersects(const glm::vec3& start, const glm::vec3& end, float& distance, glm::vec3& normal);

signals:

    void boundsWillChange();
    void boundsChanged(const Box& bounds);

protected:
    
    SpannerRenderer* _renderer;
    
    /// Returns the name of the class to instantiate in order to render this spanner.
    virtual QByteArray getRendererClassName() const;

private:
    
    Box _bounds;
    float _placementGranularity;
    float _voxelizationGranularity;
    bool _merged;
    QHash<QThread*, int> _lastVisits; ///< last visit identifiers for each thread
    QMutex _lastVisitsMutex;
    
    static QAtomicInt _nextVisit; ///< the global visit counter
};

/// Base class for objects that can render spanners.
class SpannerRenderer : public QObject {
    Q_OBJECT
    
public:
    
    enum Mode { DEFAULT_MODE, DIFFUSE_MODE, NORMAL_MODE };
    
    Q_INVOKABLE SpannerRenderer();
    
    virtual void init(Spanner* spanner);
    virtual void simulate(float deltaTime);
    virtual void render(const glm::vec4& color, Mode mode);
    virtual bool findRayIntersection(const glm::vec3& origin, const glm::vec3& direction, float& distance) const;

protected:
    
    Spanner* _spanner;
};

/// An object with a 3D transform.
class Transformable : public Spanner {
    Q_OBJECT
    Q_PROPERTY(glm::vec3 translation MEMBER _translation WRITE setTranslation NOTIFY translationChanged)
    Q_PROPERTY(glm::quat rotation MEMBER _rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(float scale MEMBER _scale WRITE setScale NOTIFY scaleChanged)

public:

    Transformable();

    void setTranslation(const glm::vec3& translation);
    const glm::vec3& getTranslation() const { return _translation; }
    
    void setRotation(const glm::quat& rotation);
    const glm::quat& getRotation() const { return _rotation; }
    
    void setScale(float scale);
    float getScale() const { return _scale; }

signals:

    void translationChanged(const glm::vec3& translation);
    void rotationChanged(const glm::quat& rotation);
    void scaleChanged(float scale);

private:
    
    glm::vec3 _translation;
    glm::quat _rotation;
    float _scale;
};

/// A transformable object with a color.
class ColorTransformable : public Transformable {
    Q_OBJECT
    Q_PROPERTY(QColor color MEMBER _color WRITE setColor NOTIFY colorChanged DESIGNABLE false)
    
public:
   
    ColorTransformable();
    
    void setColor(const QColor& color);
    const QColor& getColor() const { return _color; }

signals:

    void colorChanged(const QColor& color);
    
protected:
    
    QColor _color;
};

/// A sphere.
class Sphere : public ColorTransformable {
    Q_OBJECT
    
public:
    
    Q_INVOKABLE Sphere();

    virtual bool findRayIntersection(const glm::vec3& origin, const glm::vec3& direction, float& distance) const;
    virtual bool contains(const glm::vec3& point);
    virtual bool intersects(const glm::vec3& start, const glm::vec3& end, float& distance, glm::vec3& normal);
    
protected:
    
    virtual QByteArray getRendererClassName() const;

private slots:
    
    void updateBounds();
};

/// A cuboid.
class Cuboid : public ColorTransformable {
    Q_OBJECT
    Q_PROPERTY(float aspectY MEMBER _aspectY WRITE setAspectY NOTIFY aspectYChanged)
    Q_PROPERTY(float aspectZ MEMBER _aspectZ WRITE setAspectZ NOTIFY aspectZChanged)
    
public:
    
    Q_INVOKABLE Cuboid();

    void setAspectY(float aspectY);
    float getAspectY() const { return _aspectY; }
    
    void setAspectZ(float aspectZ);
    float getAspectZ() const { return _aspectZ; }

    virtual bool contains(const glm::vec3& point);
    virtual bool intersects(const glm::vec3& start, const glm::vec3& end, float& distance, glm::vec3& normal);

signals:

    void aspectYChanged(float aspectY);
    void aspectZChanged(float aspectZ);

protected:
    
    virtual QByteArray getRendererClassName() const;

private slots:
    
    void updateBoundsAndPlanes();
    
private:

    float _aspectY;
    float _aspectZ;
    
    static const int PLANE_COUNT = 6;
    glm::vec4 _planes[PLANE_COUNT];
};

/// A static 3D model loaded from the network.
class StaticModel : public Transformable {
    Q_OBJECT
    Q_PROPERTY(QUrl url MEMBER _url WRITE setURL NOTIFY urlChanged)

public:
    
    Q_INVOKABLE StaticModel();

    void setURL(const QUrl& url);
    const QUrl& getURL() const { return _url; }

    virtual bool findRayIntersection(const glm::vec3& origin, const glm::vec3& direction, float& distance) const;
    
signals:

    void urlChanged(const QUrl& url);

protected:
    
    virtual QByteArray getRendererClassName() const;
    
private:
    
    QUrl _url;
};

/// A heightfield represented as a spanner.
class TempHeightfield : public Transformable {
    Q_OBJECT

public:
    
    TempHeightfield(const Box& bounds, float increment, const QByteArray& height, const QByteArray& color,
        const QByteArray& material, const QVector<SharedObjectPointer>& materials);
    
    QByteArray& getHeight() { return _height; }
    QByteArray& getColor() { return _color; }
    QByteArray& getMaterial() { return _material; }
    
    virtual bool hasOwnColors() const;
    virtual bool hasOwnMaterials() const;
    virtual QRgb getColorAt(const glm::vec3& point);
    virtual int getMaterialAt(const glm::vec3& point);
    virtual QVector<SharedObjectPointer>& getMaterials();
    
    virtual bool contains(const glm::vec3& point);
    virtual bool intersects(const glm::vec3& start, const glm::vec3& end, float& distance, glm::vec3& normal);

private:
    
    float _increment;
    int _width;
    float _heightScale;
    QByteArray _height;
    QByteArray _color;
    QByteArray _material;
    QVector<SharedObjectPointer> _materials;
};

/// Base class for heightfield data blocks.
class HeightfieldData : public DataBlock {
public:
    
    static const int SHARED_EDGE = 1;
    
    HeightfieldData(int width = 0);
    
    int getWidth() const { return _width; }
    
protected:
    
    int _width;
};

typedef QExplicitlySharedDataPointer<HeightfieldHeight> HeightfieldHeightPointer;

/// A block of height data associated with a heightfield.
class HeightfieldHeight : public HeightfieldData {
public:

    static const int HEIGHT_BORDER = 1;
    static const int HEIGHT_EXTENSION = SHARED_EDGE + 2 * HEIGHT_BORDER;
    
    HeightfieldHeight(int width, const QVector<quint16>& contents);
    HeightfieldHeight(Bitstream& in, int bytes);
    HeightfieldHeight(Bitstream& in, int bytes, const HeightfieldHeightPointer& reference);
    
    const QVector<quint16>& getContents() const { return _contents; }

    void write(Bitstream& out);
    void writeDelta(Bitstream& out, const HeightfieldHeightPointer& reference);
    
private:
    
    void read(Bitstream& in, int bytes);
    
    QVector<quint16> _contents;
};

Q_DECLARE_METATYPE(HeightfieldHeightPointer)

Bitstream& operator<<(Bitstream& out, const HeightfieldHeightPointer& value);
Bitstream& operator>>(Bitstream& in, HeightfieldHeightPointer& value);

template<> void Bitstream::writeRawDelta(const HeightfieldHeightPointer& value, const HeightfieldHeightPointer& reference);
template<> void Bitstream::readRawDelta(HeightfieldHeightPointer& value, const HeightfieldHeightPointer& reference);

/// Allows editing heightfield height blocks.
class HeightfieldHeightEditor : public QWidget {
    Q_OBJECT
    Q_PROPERTY(HeightfieldHeightPointer height MEMBER _height WRITE setHeight NOTIFY heightChanged USER true)
    
public:
    
    HeightfieldHeightEditor(QWidget* parent = NULL);
    
signals:

    void heightChanged(const HeightfieldHeightPointer& height);

public slots:

    void setHeight(const HeightfieldHeightPointer& height);

private slots:
    
    void select();
    void clear();    
    
private:
    
    HeightfieldHeightPointer _height;
    
    QPushButton* _select;
    QPushButton* _clear;
};

typedef QExplicitlySharedDataPointer<HeightfieldColor> HeightfieldColorPointer;

/// A block of color data associated with a heightfield.
class HeightfieldColor : public HeightfieldData {
public:
    
    HeightfieldColor(int width, const QByteArray& contents);
    HeightfieldColor(Bitstream& in, int bytes);
    HeightfieldColor(Bitstream& in, int bytes, const HeightfieldColorPointer& reference);
    
    const QByteArray& getContents() const { return _contents; }
    
    void write(Bitstream& out);
    void writeDelta(Bitstream& out, const HeightfieldColorPointer& reference);
    
private:
    
    void read(Bitstream& in, int bytes);
    
    QByteArray _contents;
};

Q_DECLARE_METATYPE(HeightfieldColorPointer)

Bitstream& operator<<(Bitstream& out, const HeightfieldColorPointer& value);
Bitstream& operator>>(Bitstream& in, HeightfieldColorPointer& value);

template<> void Bitstream::writeRawDelta(const HeightfieldColorPointer& value, const HeightfieldColorPointer& reference);
template<> void Bitstream::readRawDelta(HeightfieldColorPointer& value, const HeightfieldColorPointer& reference);

/// Allows editing heightfield color blocks.
class HeightfieldColorEditor : public QWidget {
    Q_OBJECT
    Q_PROPERTY(HeightfieldColorPointer color MEMBER _color WRITE setColor NOTIFY colorChanged USER true)
    
public:
    
    HeightfieldColorEditor(QWidget* parent = NULL);
    
signals:

    void colorChanged(const HeightfieldColorPointer& color);

public slots:

    void setColor(const HeightfieldColorPointer& color);

private slots:
    
    void select();
    void clear();
    
private:
    
    HeightfieldColorPointer _color;
    
    QPushButton* _select;
    QPushButton* _clear;
};

typedef QExplicitlySharedDataPointer<HeightfieldMaterial> HeightfieldMaterialPointer;

/// A block of material data associated with a heightfield.
class HeightfieldMaterial : public HeightfieldData {
public:
    
    HeightfieldMaterial(int width, const QByteArray& contents, const QVector<SharedObjectPointer>& materials);
    HeightfieldMaterial(Bitstream& in, int bytes);
    HeightfieldMaterial(Bitstream& in, int bytes, const HeightfieldMaterialPointer& reference);
    
    const QByteArray& getContents() const { return _contents; }
    const QVector<SharedObjectPointer>& getMaterials() const { return _materials; }
    
    void write(Bitstream& out);
    void writeDelta(Bitstream& out, const HeightfieldMaterialPointer& reference);
    
private:
    
    void read(Bitstream& in, int bytes);
    
    QByteArray _contents;
    QVector<SharedObjectPointer> _materials;
};

Q_DECLARE_METATYPE(HeightfieldMaterialPointer)

Bitstream& operator<<(Bitstream& out, const HeightfieldMaterialPointer& value);
Bitstream& operator>>(Bitstream& in, HeightfieldMaterialPointer& value);

template<> void Bitstream::writeRawDelta(const HeightfieldMaterialPointer& value, const HeightfieldMaterialPointer& reference);
template<> void Bitstream::readRawDelta(HeightfieldMaterialPointer& value, const HeightfieldMaterialPointer& reference);

/// A heightfield represented as a spanner.
class Heightfield : public Transformable {
    Q_OBJECT
    Q_PROPERTY(float aspectY MEMBER _aspectY WRITE setAspectY NOTIFY aspectYChanged)
    Q_PROPERTY(float aspectZ MEMBER _aspectZ WRITE setAspectZ NOTIFY aspectZChanged)
    Q_PROPERTY(HeightfieldHeightPointer height MEMBER _height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(HeightfieldColorPointer color MEMBER _color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(HeightfieldMaterialPointer material MEMBER _material WRITE setMaterial NOTIFY materialChanged DESIGNABLE false)

public:
    
    Q_INVOKABLE Heightfield();

    void setAspectY(float aspectY);
    float getAspectY() const { return _aspectY; }
    
    void setAspectZ(float aspectZ);
    float getAspectZ() const { return _aspectZ; }

    void setHeight(const HeightfieldHeightPointer& height);
    const HeightfieldHeightPointer& getHeight() const { return _height; }
    
    void setColor(const HeightfieldColorPointer& color);
    const HeightfieldColorPointer& getColor() const { return _color; }
    
    void setMaterial(const HeightfieldMaterialPointer& material);
    const HeightfieldMaterialPointer& getMaterial() const { return _material; }

signals:

    void aspectYChanged(float aspectY);
    void aspectZChanged(float aspectZ);
    void heightChanged(const HeightfieldHeightPointer& height);
    void colorChanged(const HeightfieldColorPointer& color);
    void materialChanged(const HeightfieldMaterialPointer& material);
    
protected:
    
    virtual QByteArray getRendererClassName() const;

private slots:
    
    void updateBounds();
    
private:

    float _aspectY;
    float _aspectZ;
    HeightfieldHeightPointer _height;
    HeightfieldColorPointer _color;
    HeightfieldMaterialPointer _material;
};

#endif // hifi_Spanner_h
