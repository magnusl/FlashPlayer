#ifndef _SWFTYPES_H_
#define _SWFTYPES_H_

#include <cstdint>
#include <vector>
#include <avm2\CStringTable.h>

namespace swf
{
    enum {
        Flag_StateNewStyles = (1 << 4),
        Flag_StateLineStyle = (1 << 3),
        Flag_StateFillStyle1 = (1 << 2),
        Flag_StateFillStyle0 = (1 << 1),
        Flag_StateMoveTo     = (1 << 0)
    };

    struct RGBA
    {
        uint8_t     r, g, b, a;
    };

    struct RGB
    {
        uint8_t     r, g, b;
    };

    struct Color_t
    {
        union {
            RGBA        rgba;
            RGB         rgb;
        };

        uint32_t getU24() const
        {
            return (((uint32_t)rgb.r) << 16) | (((uint32_t)rgb.g) << 8) | ((uint32_t)rgb.b);
        }
    };

    /**
     * SWF rectangle record.
     * The positions are specified in twips.
     */
    struct RECT
    {
        int32_t     Xmin, Xmax, Ymin, Ymax;
    };

    /**
     * \brief   Represents a positon in 2d space.
     */
    struct Vector2i
    {
        Vector2i() : x(0), y(0)
        {
        }
        Vector2i(int a_X, int a_Y) 
        {
            x = a_X;
            y = a_Y;
        }

        Vector2i operator+(const Vector2i & a_Rhs)
        {
            Vector2i tmp;
            tmp.x = x + a_Rhs.x;
            tmp.y = y + a_Rhs.y;
            return tmp;
        }

        bool operator==(const Vector2i & a_Rhs) const
        {
            return (x == a_Rhs.x) && (y == a_Rhs.y);
        }

        bool operator != (const Vector2i & a_Rhs) const
        {
            return (x != a_Rhs.x) || (y != a_Rhs.y);
        }

        int x, y;
    };

    struct Pointer 
    {
        Vector2i    Position;   /**< The position of the pointer */
    };

    /**
     * SWF file header record.
     */
    struct FileHeader
    {
        uint32_t        FileSize;
        swf::RECT       FrameSize;
        uint16_t        FrameRate;
        uint16_t        FrameCount;
        uint8_t         Signature[3];
        uint8_t         Version;
    };

    typedef struct {
        int32_t     DeltaX, DeltaY;
    } StraightEdgeRecord;

    typedef struct {
        int32_t     ControlDeltaX, ControlDeltaY;
        int32_t     AnchorDeltaX, AnchorDeltaY;
    } CurvedEdgeRecord;

    struct StyleChangedRecord
    {
        union {
            struct {
                uint8_t     StateNewStyles  : 1;
                uint8_t     StateLineStyle  : 1;
                uint8_t     StateFillStyle1 : 1;
                uint8_t     StateFillStyle0 : 1;
                uint8_t     StateMoveTo     : 1;
            };
            uint8_t cflags;
        };

        int32_t     MoveDeltaX, MoveDeltaY;     /** only valid of StateMoveTo == 1 */
        int16_t     FillStyle0;
        int16_t     FillStyle1;
        int16_t     LineStyle;                  /** new line style */

        uint8_t     NumFillBits : 4;
        uint8_t     NumLineBits : 4;
        uint8_t     LineStyleOffset;
        uint8_t     FillStyleOffset;
    };

    struct GRADRECORD
    {
        uint8_t ratio;
        Color_t color;
    };

    /**
     * \brief   Fixed point 2x3 matrix using Q16.16
     */
    struct Matrix_Fixed2x3
    {
        union {
            uint32_t ceil[6];
            struct {
                int32_t ScaleX;
                int32_t RotateSkew0;
                int32_t RotateSkew1;
                int32_t ScaleY;
                int32_t TranslateX;
                int32_t TranslateY;
            };
        };
        uint8_t HasScale     : 1;
        uint8_t HasRotate    : 1;
    };

    struct FillStyle
    {
        uint8_t     FillStyleType;
        
        enum {
            eSOLID_RGBA,
            eLINEAR_GRADIENT,
            eRADIAL_GRADIENT,
            eFOCAL_GRADIENT,
            eREPEAT_BITMAP,
            eCLIPPED_BITMAP,
            eNON_SMOOTH_REPEAT_BITMAP,
            eNON_SMOOTH_CLIPPED_BITMAP
        } FillType;

        union {
            Color_t         color;
            struct {
                uint8_t     numControlPoints;
                GRADRECORD  controlPoint[16];
            } LinearGradient;
            struct {
                uint8_t     numControlPoints;
                GRADRECORD  controlPoint[16];
                int16_t     focalPoint;
            } FocalGradient;
            struct BitmapFill 
            {
                uint16_t        bitmapId;
                Matrix_Fixed2x3 matrix;
            } Bitmap;
        };
    };

    struct LineStyle
    {
        enum {
            LINESTYLE1,
            LINESTYLE2
        } type;

        union {
            struct {
                uint16_t    width;
                bool        alphaIncluded;
                Color_t     color;
            } LineStyle1;

            struct {
                uint16_t    Width;
                uint16_t    StartCapStyle       : 2;
                uint16_t    JoinStyle           : 2;
                uint16_t    HasFillFlag         : 1;
                uint16_t    NoHScaleFlag        : 1;
                uint16_t    NoVScaleFlag        : 1;
                uint16_t    PixelHintingFlag    : 1;
                uint16_t    NoClose             : 1;
                uint16_t    EndCapStyle         : 2;

                uint16_t    MiterLimitFactor;   /**< Valid if JoinStyle = 2 */
                Color_t     Color;              /**< Valid if HasFillFlag = 0 */
                FillStyle   FillType;           /**< Valid of HasFillFlag = 1 */
            } Linestyle2;
        };
    };

    struct ShapeRecord
    {
        enum {
            END_SHAPE_RECORD,
            STYLE_CHANGE_RECORD,
            STRAIGHT_EDGE_RECORD,
            CURVED_EDGE_RECORD,
            NEW_SHAPE_RECORD
        } m_RecordType;

        union {
            StraightEdgeRecord      StraightEdge;
            CurvedEdgeRecord        CurvedEdge;
            StyleChangedRecord      StyleChanged;
        } u;

        uint8_t     NumFillBits : 4;
        uint8_t     NumLineBits : 4;
        uint8_t     LineStyleOffset;
        uint8_t     FillStyleOffset;
    };

    struct ShapeWithStyle
    {
        std::vector<FillStyle>      FillStyles;
        std::vector<LineStyle>      LineStyles;

        std::vector<ShapeRecord>    Shapes;
    };

    /**
     * Used for DefineShape, DefineShape2 and DefineShape3.
     */
    struct DefinedShape
    {
        uint16_t        ShapeId;
        RECT            ShapeBounds;
        ShapeWithStyle  Shapes;
    };

    struct DefinedShape4
    {
        uint16_t                    ShapeId;
        RECT                        ShapeBounds;
        RECT                        EdgesBounds;
        ShapeWithStyle              Shapes;
        uint8_t                     UsesFillWindingRule     : 1;
        uint8_t                     UsesNonScalingStrokes   : 1;
        uint8_t                     UsesScalingStrokes      : 1;
    };

    /**
     * \brief   SHAPE is used by the DefineFont tag, to define character glyphs.
     * \detals  Does not include fill style or line style information.
     */
    struct Shape
    {
        uint8_t     NumFillBits : 4;
        uint8_t     NumLineBits : 4;
        std::vector<ShapeRecord>    Shapes;
    };

    /*************************************************************************/
    /*                              Shape Morphing                           */
    /*************************************************************************/

    struct MorphFillStyle
    {
        uint8_t     FillStyleType;
        union {
            struct {
                RGBA        StartColor;
                RGBA        EndColor;
            } solid;
        };
    };

    struct MorphLineStyle
    {
        uint8_t IsVerision2 : 1;
        union {
            struct {
                uint16_t StartWidth;
                uint16_t EndWidth;
                RGBA     StartColor;
                RGBA     EndColor;
            } version1;
            struct {
                uint16_t StartWidth;
                uint16_t EndWidth;

                uint16_t StartCapStyle      : 2;
                uint16_t JoinStyle          : 2;
                uint16_t HasFillFlag        : 1;
                uint16_t NoHScaleFlag       : 1;
                uint16_t NoVScaleFlag       : 1;
                uint16_t PixelHintingFlag   : 1;
                uint16_t NoClose            : 1;
                uint16_t EndCapStyle        : 2;

                uint16_t MiterLimitFactor;      /**< if joinstyle = 2 */
                RGBA     StartColor;            /**< if HasFillFlag = 0 */
                RGBA     EndColor;              /**< if HasFillFlag = 0 */
                MorphFillStyle FillType;        /**< if HasFillFlag = 1 */
            } version2;
        };
    };

    struct MorphShape2
    {
        RECT        StartBounds;
        RECT        EndBounds;
        RECT        StartEdgeBounds;
        RECT        EndEdgeBounds;
        uint8_t     UseNonScalingStrokes    : 1;
        uint8_t     UsesScalingStrokes      : 1;

        std::vector<MorphFillStyle> FillStyles;
        std::vector<MorphLineStyle> LineStyles;
        Shape       StartEdges;
        Shape       EndEdges;
    };

#if 0
    typedef enum {
        eU29,       /** 29 bit unsigned integer */
        eI29,       /** 29 bit signed integer */
        eString,    /** Index into the string pool */
        eNumber,    /** IEEE-754 double-precision floating-point number */
        eObject,    /** general purpose object */
        eNull,
        eUndefined
    } TypeTag_t;

    typedef enum {
        eERR_OK,
        eERR_NOT_FOUND,
        eERR_READ_ONLY,
        eERR_EOF,
        eERR_CHAR_DEF_SPRITE,
    } Error_t;
#endif
    struct CXFormWithAlpha
    {
        uint8_t HasAddTerms : 1;
        uint8_t HasMultTerms : 1;
        int32_t RedMultTerm;
        int32_t GreenMultTerm;
        int32_t BlueMultTerm;
        int32_t AlphaMultTerm;
        int32_t RedAddTerm;
        int32_t GreenAddTerm;
        int32_t BlueAddTerm;
        int32_t AlphaAddTerm;
    };

    struct DropShadowFilter
    {
        swf::RGBA   DropShadowColor;
        int32_t     BlurX;
        int32_t     BlurY;
        int32_t     Angle;
        int32_t     Distance;
        int32_t     Strength;
        uint8_t     InnerShadow     : 1;
        uint8_t     Knockout        : 1;
        uint8_t     CompositeSource : 1;
        uint8_t     Passes          : 5;
    };

    struct BlurFilter
    {
        int32_t     BlurX;
        int32_t     BlurY;
        uint8_t     Passes : 5;
    };

    struct GlowFilter
    {
        RGBA glowColor;
        int32_t BlurX;
        int32_t BlurY;
        int16_t Strength;
        uint8_t InnerGlow : 1;
        uint8_t KnockOut : 1;
        uint8_t CompositeSource : 1;
        uint8_t Passes : 3;
    };

    struct ColorMatrixFilter
    {
        float Matrix[20];
    };

    struct Filter
    {
        uint8_t     Type;
        union {
            GlowFilter          glow;
            DropShadowFilter    dropShadow;
            BlurFilter          blur;
            ColorMatrixFilter   colorMatrix;
        };
    };

    struct PlaceObject_t
    {
        uint16_t            CharacterId;
        uint16_t            Depth;
        Matrix_Fixed2x3     TransformationMatrix;
    };

    struct PlaceObject2_t
    {
        uint8_t         PlaceFlagHasClipActions : 1;
        uint8_t         PlaceFlagHasClipDepth : 1;
        uint8_t         PlaceFlagHasName : 1;
        uint8_t         PlaceFlagHasRatio : 1;
        uint8_t         PlaceFlagHasColorTransform : 1;
        uint8_t         PlaceFlagHasMatrix : 1;
        uint8_t         PlaceFlagHasCharacter : 1;
        uint8_t         PlaceFlagMove : 1;

        uint16_t            CharacterId;
        uint16_t            Depth;
        Matrix_Fixed2x3     TransformationMatrix;
        uint16_t            Ratio;
        uint16_t            ClipDepth;
        CXFormWithAlpha     cxform;

        avm2::CStringTable::index_t Name;
    };

    struct PlaceObject3_t
    {
        uint8_t         PlaceFlagHasClipActions : 1;
        uint8_t         PlaceFlagHasClipDepth : 1;
        uint8_t         PlaceFlagHasName : 1;
        uint8_t         PlaceFlagHasRatio : 1;
        uint8_t         PlaceFlagHasColorTransform : 1;
        uint8_t         PlaceFlagHasMatrix : 1;
        uint8_t         PlaceFlagHasCharacter : 1;
        uint8_t         PlaceFlagMove : 1;
        uint8_t         Reserved : 1;
        uint8_t         PlaceFlagOpaqueBackground : 1;
        uint8_t         PlaceFlagHasVisible : 1;
        uint8_t         PlaceFlagHasImage : 1;
        uint8_t         PlaceFlagHasClassName : 1;
        uint8_t         PlaceFlagHasCacheAsBitmap : 1;
        uint8_t         PlaceFlagHasBlendMode : 1;
        uint8_t         PlaceFlagHasFilterList : 1;

        uint16_t            Depth;
        uint16_t            CharacterId;
        Matrix_Fixed2x3     TransformationMatrix;
        uint16_t            Ratio;
        uint16_t            ClipDepth;
        CXFormWithAlpha     cxform;
        avm2::CStringTable::index_t Name;
        avm2::CStringTable::index_t ClassName;
        uint8_t             BitmapCache;
        uint8_t             Visible;
        RGBA                BackgroundColor;
        uint8_t             BlendMode;
        uint8_t             NumFilters;
        Filter              Filters[8];
    };

    struct RemoveObject_t
    {
        uint16_t    CharacterId;
        uint16_t    Depth;
    };

    struct RemoveObject2_t
    {
        uint16_t    Depth;
    };

    struct Buttonrecord_t
    {
        uint8_t ButtonHasBlendMode : 1;
        uint8_t ButtonHasFilterList : 1;
        uint8_t ButtonStateHitTest : 1;
        uint8_t ButtonStateDown : 1;
        uint8_t ButtonStateOver : 1;
        uint8_t ButtonStateUp : 1;

        uint16_t            CharacterID;
        uint16_t            PlaceDepth;
        Matrix_Fixed2x3     Matrix;
        CXFormWithAlpha     CxForm;
        uint8_t             BlendMode;
    };

    struct Button_t
    {
        std::vector<Buttonrecord_t> Records;
        bool                        TrackAsMenu;
    };
}

#endif