using System;

namespace Louron
{
    public struct Vector2
    {
        public float X, Y;

        public static Vector2 Zero => new Vector2(0.0f);

        public Vector2(float scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        #region Operators

        // Add
        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X + b.X, a.Y + b.Y);
        }

        public static Vector2 operator +(Vector2 a, float scalar)
        {
            return new Vector2(a.X + scalar, a.Y + scalar);
        }

        public static Vector2 operator +(float scalar, Vector2 a)
        {
            return new Vector2(a.X + scalar, a.Y + scalar);
        }

        // Subtract
        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X - b.X, a.Y - b.Y);
        }

        public static Vector2 operator -(Vector2 a, float scalar)
        {
            return new Vector2(a.X - scalar, a.Y - scalar);
        }

        public static Vector2 operator -(float scalar, Vector2 a)
        {
            return new Vector2(a.X - scalar, a.Y - scalar);
        }

        // Multiply
        public static Vector2 operator *(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X * b.X, a.Y * b.Y);
        }

        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.X * scalar, vector.Y * scalar);
        }

        public static Vector2 operator *(float scalar, Vector2 vector)
        {
            return new Vector2(vector.X * scalar, vector.Y * scalar);
        }

        // Divide
        public static Vector2 operator /(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X / b.X, a.Y / b.Y);
        }

        public static Vector2 operator /(Vector2 vector, float scalar)
        {
            return new Vector2(vector.X / scalar, vector.Y / scalar);
        }

        public static Vector2 operator /(float scalar, Vector2 vector)
        {
            return new Vector2(vector.X / scalar, vector.Y / scalar);
        }

        public static bool operator ==(Vector2 a, Vector2 b)
        {
            return a.X == b.X && a.Y == b.Y;
        }

        public static bool operator !=(Vector2 a, Vector2 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods
        /// <summary>
        /// Calculates the distance between two Vector2 points.
        /// </summary>
        /// <param name="a">The first vector.</param>
        /// <param name="b">The second vector.</param>
        /// <returns>Distance between two Vectors.</returns>
        public static float Distance(Vector2 a, Vector2 b)
        {
            float dx = a.X - b.X;
            float dy = a.Y - b.Y;
            return MathF.Sqrt(dx * dx + dy * dy);
        }

        /// <summary>
        /// Performs linear interpolation between two Vector2 points.
        /// </summary>
        /// <param name="t">
        /// Interpolation factor, typically between 0 and 1.
        /// A value of 0 returns the first vector, and a value of 1 returns the second vector.
        /// </param>
        /// <param name="a">The starting vector.</param>
        /// <param name="b">The target vector.</param>
        /// <returns>A Vector2 that is the linear interpolation between <paramref name="a"/> and <paramref name="b"/>.</returns>
        public static Vector2 Lerp(float t, Vector2 a, Vector2 b)
        {
            return new Vector2(
                LMath.Lerp(t, a.X, b.X),
                LMath.Lerp(t, a.Y, b.Y)
            );
        }

        /// <summary>
        /// Performs spherical linear interpolation between two Vector2 directions.
        /// </summary>
        /// <param name="t">
        /// Interpolation factor, typically between 0 and 1.
        /// A value of 0 returns the first vector, and a value of 1 returns the second vector.
        /// </param>
        /// <param name="a">The starting direction vector. Should be normalized.</param>
        /// <param name="b">The target direction vector. Should be normalized.</param>
        /// <returns>
        /// A Vector2 that is the spherical linear interpolation between <paramref name="a"/> and <paramref name="b"/>.
        /// </returns>
        /// <remarks>
        /// Ensure that both vectors are normalized before calling this function.
        /// If the input vectors are collinear or one is zero, the result may be undefined.
        /// </remarks>
        public static Vector2 Slerp(float t, Vector2 a, Vector2 b)
        {
            Vector2 na = Normalize(a);
            Vector2 nb = Normalize(b);

            float dot = LMath.Dot(na, nb);
            dot = Math.Clamp(dot, -1.0f, 1.0f);
            float theta = MathF.Acos(dot) * t;

            Vector2 relativeVec = nb - na * dot;
            relativeVec = Normalize(relativeVec);

            return na * MathF.Cos(theta) + relativeVec * MathF.Sin(theta);
        }

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is Vector2 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y);
        }

        // Normalize function
        public readonly Vector2 Normalize()
        {
            float length = (float)Math.Sqrt(X * X + Y * Y);
            if (length > 0)
            {
                return this * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Static Normalize function
        public static Vector2 Normalize(Vector2 v)
        {
            float length = (float)Math.Sqrt(v.X * v.X + v.Y * v.Y);
            if (length > 0)
            {
                return v * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"Vector2({X}, {Y})";
        }
        #endregion
    }

    public struct IVector2
    {
        public int X, Y;

        public static IVector2 Zero => new IVector2(0);

        public IVector2(int scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public IVector2(int x, int y)
        {
            X = x;
            Y = y;
        }

        #region Operators

        // Add
        public static IVector2 operator +(IVector2 a, IVector2 b)
        {
            return new IVector2(a.X + b.X, a.Y + b.Y);
        }

        public static IVector2 operator +(IVector2 a, int scalar)
        {
            return new IVector2(a.X + scalar, a.Y + scalar);
        }

        public static IVector2 operator +(int scalar, IVector2 a)
        {
            return new IVector2(a.X + scalar, a.Y + scalar);
        }

        // Subtract
        public static IVector2 operator -(IVector2 a, IVector2 b)
        {
            return new IVector2(a.X - b.X, a.Y - b.Y);
        }

        public static IVector2 operator -(IVector2 a, int scalar)
        {
            return new IVector2(a.X - scalar, a.Y - scalar);
        }

        public static IVector2 operator -(int scalar, IVector2 a)
        {
            return new IVector2(a.X - scalar, a.Y - scalar);
        }

        // Multiply
        public static IVector2 operator *(IVector2 a, IVector2 b)
        {
            return new IVector2(a.X * b.X, a.Y * b.Y);
        }

        public static IVector2 operator *(IVector2 vector, int scalar)
        {
            return new IVector2(vector.X * scalar, vector.Y * scalar);
        }

        public static IVector2 operator *(int scalar, IVector2 vector)
        {
            return new IVector2(vector.X * scalar, vector.Y * scalar);
        }

        // Divide
        public static IVector2 operator /(IVector2 a, IVector2 b)
        {
            return new IVector2(a.X / b.X, a.Y / b.Y);
        }

        public static IVector2 operator /(IVector2 vector, int scalar)
        {
            return new IVector2(vector.X / scalar, vector.Y / scalar);
        }

        public static IVector2 operator /(int scalar, IVector2 vector)
        {
            return new IVector2(vector.X / scalar, vector.Y / scalar);
        }

        public static bool operator ==(IVector2 a, IVector2 b)
        {
            return a.X == b.X && a.Y == b.Y;
        }

        public static bool operator !=(IVector2 a, IVector2 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is IVector2 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"IVector2({X}, {Y})";
        }
        #endregion
    }

    public struct  UVector2
    {
        public uint X, Y;

        public static  UVector2 Zero => new  UVector2(0);

        public  UVector2(uint scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public  UVector2(uint x, uint y)
        {
            X = x;
            Y = y;
        }

        #region Operators

        // Add
        public static  UVector2 operator +( UVector2 a,  UVector2 b)
        {
            return new  UVector2(a.X + b.X, a.Y + b.Y);
        }

        public static  UVector2 operator +( UVector2 a, uint scalar)
        {
            return new  UVector2(a.X + scalar, a.Y + scalar);
        }

        public static  UVector2 operator +(uint scalar,  UVector2 a)
        {
            return new  UVector2(a.X + scalar, a.Y + scalar);
        }

        // Subtract
        public static  UVector2 operator -( UVector2 a,  UVector2 b)
        {
            return new  UVector2(a.X - b.X, a.Y - b.Y);
        }

        public static  UVector2 operator -( UVector2 a, uint scalar)
        {
            return new  UVector2(a.X - scalar, a.Y - scalar);
        }

        public static  UVector2 operator -(uint scalar,  UVector2 a)
        {
            return new  UVector2(a.X - scalar, a.Y - scalar);
        }

        // Multiply
        public static  UVector2 operator *( UVector2 a,  UVector2 b)
        {
            return new  UVector2(a.X * b.X, a.Y * b.Y);
        }

        public static  UVector2 operator *( UVector2 vector, uint scalar)
        {
            return new  UVector2(vector.X * scalar, vector.Y * scalar);
        }

        public static  UVector2 operator *(uint scalar,  UVector2 vector)
        {
            return new  UVector2(vector.X * scalar, vector.Y * scalar);
        }

        // Divide
        public static  UVector2 operator /( UVector2 a,  UVector2 b)
        {
            return new  UVector2(a.X / b.X, a.Y / b.Y);
        }

        public static  UVector2 operator /( UVector2 vector, uint scalar)
        {
            return new  UVector2(vector.X / scalar, vector.Y / scalar);
        }

        public static  UVector2 operator /(uint scalar,  UVector2 vector)
        {
            return new  UVector2(vector.X / scalar, vector.Y / scalar);
        }

        public static bool operator ==( UVector2 a,  UVector2 b)
        {
            return a.X == b.X && a.Y == b.Y;
        }

        public static bool operator !=( UVector2 a,  UVector2 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is  UVector2 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"UVector2({X}, {Y})";
        }
        #endregion
    }

    public struct DVector2
    {
        public double X, Y;

        public static DVector2 Zero => new DVector2(0);

        public DVector2(double scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public DVector2(double x, double y)
        {
            X = x;
            Y = y;
        }

        #region Operators

        // Add
        public static DVector2 operator +(DVector2 a, DVector2 b)
        {
            return new DVector2(a.X + b.X, a.Y + b.Y);
        }

        public static DVector2 operator +(DVector2 a, double scalar)
        {
            return new DVector2(a.X + scalar, a.Y + scalar);
        }

        public static DVector2 operator +(double scalar, DVector2 a)
        {
            return new DVector2(a.X + scalar, a.Y + scalar);
        }

        // Subtract
        public static DVector2 operator -(DVector2 a, DVector2 b)
        {
            return new DVector2(a.X - b.X, a.Y - b.Y);
        }

        public static DVector2 operator -(DVector2 a, double scalar)
        {
            return new DVector2(a.X - scalar, a.Y - scalar);
        }

        public static DVector2 operator -(double scalar, DVector2 a)
        {
            return new DVector2(a.X - scalar, a.Y - scalar);
        }

        // Multiply
        public static DVector2 operator *(DVector2 a, DVector2 b)
        {
            return new DVector2(a.X * b.X, a.Y * b.Y);
        }

        public static DVector2 operator *(DVector2 vector, double scalar)
        {
            return new DVector2(vector.X * scalar, vector.Y * scalar);
        }

        public static DVector2 operator *(double scalar, DVector2 vector)
        {
            return new DVector2(vector.X * scalar, vector.Y * scalar);
        }

        // Divide
        public static DVector2 operator /(DVector2 a, DVector2 b)
        {
            return new DVector2(a.X / b.X, a.Y / b.Y);
        }

        public static DVector2 operator /(DVector2 vector, double scalar)
        {
            return new DVector2(vector.X / scalar, vector.Y / scalar);
        }

        public static DVector2 operator /(double scalar, DVector2 vector)
        {
            return new DVector2(vector.X / scalar, vector.Y / scalar);
        }

        public static bool operator ==(DVector2 a, DVector2 b)
        {
            return a.X == b.X && a.Y == b.Y;
        }

        public static bool operator !=(DVector2 a, DVector2 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is DVector2 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"DVector2({X}, {Y})";
        }
        #endregion
    }

    public struct BVector2
    {
        public bool X, Y;

        public static BVector2 Zero => new BVector2(false);

        public BVector2(bool scalar)
        {
            X = scalar;
            Y = scalar;
        }

        public BVector2(bool x, bool y)
        {
            X = x;
            Y = y;
        }

        #region Operators

        public static bool operator ==(BVector2 a, BVector2 b)
        {
            return a.X == b.X && a.Y == b.Y;
        }

        public static bool operator !=(BVector2 a, BVector2 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is BVector2 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"BVector2({X}, {Y})";
        }
        #endregion
    }

}