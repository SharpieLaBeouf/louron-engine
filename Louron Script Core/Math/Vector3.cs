using System.Numerics;

namespace Louron
{
    public struct Vector3
    {
        public float X, Y, Z;

        public static Vector3 Zero => new Vector3(0.0f);
        public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
        public static Vector3 Front => new Vector3(0.0f, 0.0f, -1.0f); // Negative Z-axis
        public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);  // Positive X-axis

        public Vector3(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public Vector2 XY
        {
            get => new Vector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        #region Operators

        // Add
        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static Vector3 operator +(Vector3 a, float scalar)
        {
            return new Vector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        public static Vector3 operator +(float scalar, Vector3 a)
        {
            return new Vector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        // Subtract
        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        }

        public static Vector3 operator -(Vector3 a, float scalar)
        {
            return new Vector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        public static Vector3 operator -(float scalar, Vector3 a)
        {
            return new Vector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        // Multiply
        public static Vector3 operator *(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar  );
        }

        public static Vector3 operator *(float scalar, Vector3 vector)
        {
            return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        // Divide
        public static Vector3 operator /(Vector3 a, Vector3 b)
        {
            return new Vector3(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
        }

        public static Vector3 operator /(Vector3 vector, float scalar)
        {
            return new Vector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static Vector3 operator /(float scalar, Vector3 vector)
        {
            return new Vector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static bool operator ==(Vector3 a, Vector3 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
        }

        public static bool operator !=(Vector3 a, Vector3 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Calculates the distance between two Vector3 points.
        /// </summary>
        /// <param name="a">The first vector.</param>
        /// <param name="b">The second vector.</param>
        /// <returns>Distance between two Vectors.</returns>
        public static float Distance(Vector3 a, Vector3 b)
        {
            float dx = a.X - b.X;
            float dy = a.Y - b.Y;
            float dz = a.Z - b.Z;
            return MathF.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        /// <summary>
        /// Performs linear interpolation between two Vector3 points.
        /// </summary>
        /// <param name="t">
        /// Interpolation factor, typically between 0 and 1.
        /// A value of 0 returns the first vector, and a value of 1 returns the second vector.
        /// </param>
        /// <param name="a">The starting vector.</param>
        /// <param name="b">The target vector.</param>
        /// <returns>A Vector3 that is the linear interpolation between <paramref name="a"/> and <paramref name="b"/>.</returns>
        public static Vector3 Lerp(float t, Vector3 a, Vector3 b)
        {
            return new Vector3(
                LMath.Lerp(t, a.X, b.X),
                LMath.Lerp(t, a.Y, b.Y),
                LMath.Lerp(t, a.Z, b.Z)
            );
        }

        /// <summary>
        /// Performs spherical linear interpolation between two Vector3 directions.
        /// </summary>
        /// <param name="t">
        /// Interpolation factor, typically between 0 and 1.
        /// A value of 0 returns the first vector, and a value of 1 returns the second vector.
        /// </param>
        /// <param name="a">The starting direction vector. Should be normalized.</param>
        /// <param name="b">The target direction vector. Should be normalized.</param>
        /// <returns>
        /// A Vector3 that is the spherical linear interpolation between <paramref name="a"/> and <paramref name="b"/>.
        /// </returns>
        /// <remarks>
        /// Ensure that both vectors are normalized before calling this function.
        /// If the input vectors are collinear or one is zero, the result may be undefined.
        /// </remarks>
        public static Vector3 Slerp(float t, Vector3 a, Vector3 b)
        {
            // Normalize input vectors to ensure they represent directions
            Vector3 na = Normalize(a);
            Vector3 nb = Normalize(b);

            // Calculate the angle between the vectors
            float dot = LMath.Dot(na, nb);
            dot = Math.Clamp(dot, -1.0f, 1.0f); // Clamp to avoid numerical errors
            float theta = MathF.Acos(dot) * t;

            // Calculate the perpendicular vector
            Vector3 relativeVec = nb - na * dot;
            relativeVec = Normalize(relativeVec);

            // Perform the interpolation
            return na * MathF.Cos(theta) + relativeVec * MathF.Sin(theta);
        }

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is Vector3 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z);
        }

        // Normalize function
        public readonly Vector3 Normalize()
        {
            float length = (float)Math.Sqrt(X * X + Y * Y + Z * Z);
            if (length > 0)
            {
                return this * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Static Normalize function
        public static Vector3 Normalize(Vector3 v)
        {
            float length = (float)Math.Sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
            if (length > 0)
            {
                return v * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"Vector3({X}, {Y}, {Z})";
        }

        #endregion
    }

    public struct IVector3
    {
        public int X, Y, Z;

        public static IVector3 Zero => new IVector3(0);

        public IVector3(int scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public IVector3(int x, int y, int z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public IVector3(IVector2 xy, int z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public IVector2 XY
        {
            get => new IVector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        #region Operators

        // Add
        public static IVector3 operator +(IVector3 a, IVector3 b)
        {
            return new IVector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static IVector3 operator +(IVector3 a, int scalar)
        {
            return new IVector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        public static IVector3 operator +(int scalar, IVector3 a)
        {
            return new IVector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        // Subtract
        public static IVector3 operator -(IVector3 a, IVector3 b)
        {
            return new IVector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        }

        public static IVector3 operator -(IVector3 a, int scalar)
        {
            return new IVector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        public static IVector3 operator -(int scalar, IVector3 a)
        {
            return new IVector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        // Multiply
        public static IVector3 operator *(IVector3 a, IVector3 b)
        {
            return new IVector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
        }

        public static IVector3 operator *(IVector3 vector, int scalar)
        {
            return new IVector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        public static IVector3 operator *(int scalar, IVector3 vector)
        {
            return new IVector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        // Divide
        public static IVector3 operator /(IVector3 a, IVector3 b)
        {
            return new IVector3(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
        }

        public static IVector3 operator /(IVector3 vector, int scalar)
        {
            return new IVector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static IVector3 operator /(int scalar, IVector3 vector)
        {
            return new IVector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static bool operator ==(IVector3 a, IVector3 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
        }

        public static bool operator !=(IVector3 a, IVector3 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is IVector3 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"IVector3({X}, {Y}, {Z})";
        }

        #endregion
    }

    public struct UVector3
    {
        public uint X, Y, Z;

        public static UVector3 Zero => new UVector3(0);

        public UVector3(uint scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public UVector3(uint x, uint y, uint z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public UVector3(UVector2 xy, uint z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public UVector2 XY
        {
            get => new UVector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        #region Operators

        // Add
        public static UVector3 operator +(UVector3 a, UVector3 b)
        {
            return new UVector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static UVector3 operator +(UVector3 a, uint scalar)
        {
            return new UVector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        public static UVector3 operator +(uint scalar, UVector3 a)
        {
            return new UVector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        // Subtract
        public static UVector3 operator -(UVector3 a, UVector3 b)
        {
            return new UVector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        }

        public static UVector3 operator -(UVector3 a, uint scalar)
        {
            return new UVector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        public static UVector3 operator -(uint scalar, UVector3 a)
        {
            return new UVector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        // Multiply
        public static UVector3 operator *(UVector3 a, UVector3 b)
        {
            return new UVector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
        }

        public static UVector3 operator *(UVector3 vector, uint scalar)
        {
            return new UVector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        public static UVector3 operator *(uint scalar, UVector3 vector)
        {
            return new UVector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        // Divide
        public static UVector3 operator /(UVector3 a, UVector3 b)
        {
            return new UVector3(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
        }

        public static UVector3 operator /(UVector3 vector, uint scalar)
        {
            return new UVector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static UVector3 operator /(uint scalar, UVector3 vector)
        {
            return new UVector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static bool operator ==(UVector3 a, UVector3 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
        }

        public static bool operator !=(UVector3 a, UVector3 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is UVector3 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"UVector3({X}, {Y}, {Z})";
        }

        #endregion
    }

    public struct DVector3
    {
        public double X, Y, Z;

        public static DVector3 Zero => new DVector3(0.0);

        public DVector3(double scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public DVector3(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public DVector3(DVector2 xy, double z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public DVector2 XY
        {
            get => new DVector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        #region Operators

        // Add
        public static DVector3 operator +(DVector3 a, DVector3 b)
        {
            return new DVector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static DVector3 operator +(DVector3 a, double scalar)
        {
            return new DVector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        public static DVector3 operator +(double scalar, DVector3 a)
        {
            return new DVector3(a.X + scalar, a.Y + scalar, a.Z + scalar);
        }

        // Subtract
        public static DVector3 operator -(DVector3 a, DVector3 b)
        {
            return new DVector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        }

        public static DVector3 operator -(DVector3 a, double scalar)
        {
            return new DVector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        public static DVector3 operator -(double scalar, DVector3 a)
        {
            return new DVector3(a.X - scalar, a.Y - scalar, a.Z - scalar);
        }

        // Multiply
        public static DVector3 operator *(DVector3 a, DVector3 b)
        {
            return new DVector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
        }

        public static DVector3 operator *(DVector3 vector, double scalar)
        {
            return new DVector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        public static DVector3 operator *(double scalar, DVector3 vector)
        {
            return new DVector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
        }

        // Divide
        public static DVector3 operator /(DVector3 a, DVector3 b)
        {
            return new DVector3(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
        }

        public static DVector3 operator /(DVector3 vector, double scalar)
        {
            return new DVector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static DVector3 operator /(double scalar, DVector3 vector)
        {
            return new DVector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
        }

        public static bool operator ==(DVector3 a, DVector3 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
        }

        public static bool operator !=(DVector3 a, DVector3 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is DVector3 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"DVector3({X}, {Y}, {Z})";
        }

        #endregion
    }


    public struct BVector3
    {
        public bool X, Y, Z;

        public static BVector3 Zero => new BVector3(false);

        public BVector3(bool scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
        }

        public BVector3(bool x, bool y, bool z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public BVector3(BVector2 xy, bool z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }

        public BVector2 XY
        {
            get => new BVector2(X, Y);
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        #region Operators

        public static bool operator ==(BVector3 a, BVector3 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
        }

        public static bool operator !=(BVector3 a, BVector3 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is BVector3 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"BVector3({X}, {Y}, {Z})";
        }

        #endregion
    }

}
