namespace Louron
{
    public struct Vector4
    {
        public float X, Y, Z, W;

        public static Vector4 Zero => new Vector4(0.0f);

        public Vector4(float scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public Vector4(Vector3 xyz, float w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
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

        public Vector3 XYZ
        {
            get => new Vector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        #region Operators

        // Add
        public static Vector4 operator +(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        }

        public static Vector4 operator +(Vector4 a, float scalar)
        {
            return new Vector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        public static Vector4 operator +(float scalar, Vector4 a)
        {
            return new Vector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        // Subtract
        public static Vector4 operator -(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        }

        public static Vector4 operator -(Vector4 a, float scalar)
        {
            return new Vector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        public static Vector4 operator -(float scalar, Vector4 a)
        {
            return new Vector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        // Multiply
        public static Vector4 operator *(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X * b.X, a.Y * b.Y, a.Z * b.Z, a.W * b.W);
        }

        public static Vector4 operator *(Vector4 vector, float scalar)
        {
            return new Vector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        public static Vector4 operator *(float scalar, Vector4 vector)
        {
            return new Vector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        // Divide
        public static Vector4 operator /(Vector4 a, Vector4 b)
        {
            return new Vector4(a.X / b.X, a.Y / b.Y, a.Z / b.Z, a.W / b.W);
        }

        public static Vector4 operator /(Vector4 vector, float scalar)
        {
            return new Vector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        public static Vector4 operator /(float scalar, Vector4 vector)
        {
            return new Vector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        // Equality
        public static bool operator ==(Vector4 a, Vector4 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;
        }

        public static bool operator !=(Vector4 a, Vector4 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods
        /// <summary>
        /// Calculates the distance between two Vector4 points.
        /// </summary>
        /// <param name="a">The first vector.</param>
        /// <param name="b">The second vector.</param>
        /// <returns>Distance between two Vectors.</returns>
        public static float Distance(Vector4 a, Vector4 b)
        {
            float dx = a.X - b.X;
            float dy = a.Y - b.Y;
            float dz = a.Z - b.Z;
            float dw = a.W - b.W;
            return MathF.Sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
        }

        /// <summary>
        /// Performs linear interpolation between two Vector4 points.
        /// </summary>
        /// <param name="t">
        /// Interpolation factor, typically between 0 and 1.
        /// A value of 0 returns the first vector, and a value of 1 returns the second vector.
        /// </param>
        /// <param name="a">The starting vector.</param>
        /// <param name="b">The target vector.</param>
        /// <returns>A Vector4 that is the linear interpolation between <paramref name="a"/> and <paramref name="b"/>.</returns>
        public static Vector4 Lerp(float t, Vector4 a, Vector4 b)
        {
            return new Vector4(
                LMath.Lerp(t, a.X, b.X),
                LMath.Lerp(t, a.Y, b.Y),
                LMath.Lerp(t, a.Z, b.Z),
                LMath.Lerp(t, a.W, b.W)
            );
        }

        /// <summary>
        /// Performs spherical linear interpolation between two Vector4 directions.
        /// </summary>
        /// <param name="t">
        /// Interpolation factor, typically between 0 and 1.
        /// A value of 0 returns the first vector, and a value of 1 returns the second vector.
        /// </param>
        /// <param name="a">The starting direction vector. Should be normalized.</param>
        /// <param name="b">The target direction vector. Should be normalized.</param>
        /// <returns>
        /// A Vector4 that is the spherical linear interpolation between <paramref name="a"/> and <paramref name="b"/>.
        /// </returns>
        /// <remarks>
        /// Ensure that both vectors are normalized before calling this function.
        /// If the input vectors are collinear or one is zero, the result may be undefined.
        /// </remarks>
        public static Vector4 Slerp(float t, Vector4 a, Vector4 b)
        {
            Vector4 na = Normalize(a);
            Vector4 nb = Normalize(b);

            float dot = LMath.Dot(na, nb);
            dot = Math.Clamp(dot, -1.0f, 1.0f);
            float theta = MathF.Acos(dot) * t;

            Vector4 relativeVec = nb - na * dot;
            relativeVec = Normalize(relativeVec);

            return na * MathF.Cos(theta) + relativeVec * MathF.Sin(theta);
        }

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is Vector4 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z, W);
        }

        // Normalize function
        public readonly Vector4 Normalize()
        {
            float length = (float)Math.Sqrt(X * X + Y * Y + Z * Z + W * W);
            if (length > 0)
            {
                return this * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Static Normalize function
        public static Vector4 Normalize(Vector4 v)
        {
            float length = (float)Math.Sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z + v.W * v.W);
            if (length > 0)
            {
                return v * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"Vector4({X}, {Y}, {Z}, {W})";
        }

        #endregion

    }

    public struct IVector4
    {
        public int X, Y, Z, W;

        public static IVector4 Zero => new IVector4(0);

        public IVector4(int scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public IVector4(int x, int y, int z, int w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public IVector4(IVector3 xyz, int w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
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

        public IVector3 XYZ
        {
            get => new IVector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        #region Operators

        // Add
        public static IVector4 operator +(IVector4 a, IVector4 b)
        {
            return new IVector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        }

        public static IVector4 operator +(IVector4 a, int scalar)
        {
            return new IVector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        public static IVector4 operator +(int scalar, IVector4 a)
        {
            return new IVector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        // Subtract
        public static IVector4 operator -(IVector4 a, IVector4 b)
        {
            return new IVector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        }

        public static IVector4 operator -(IVector4 a, int scalar)
        {
            return new IVector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        public static IVector4 operator -(int scalar, IVector4 a)
        {
            return new IVector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        // Multiply
        public static IVector4 operator *(IVector4 a, IVector4 b)
        {
            return new IVector4(a.X * b.X, a.Y * b.Y, a.Z * b.Z, a.W * b.W);
        }

        public static IVector4 operator *(IVector4 vector, int scalar)
        {
            return new IVector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        public static IVector4 operator *(int scalar, IVector4 vector)
        {
            return new IVector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        // Divide
        public static IVector4 operator /(IVector4 a, IVector4 b)
        {
            return new IVector4(a.X / b.X, a.Y / b.Y, a.Z / b.Z, a.W / b.W);
        }

        public static IVector4 operator /(IVector4 vector, int scalar)
        {
            return new IVector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        public static IVector4 operator /(int scalar, IVector4 vector)
        {
            return new IVector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        // Equality
        public static bool operator ==(IVector4 a, IVector4 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;
        }

        public static bool operator !=(IVector4 a, IVector4 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is IVector4 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z, W);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"IVector4({X}, {Y}, {Z}, {W})";
        }

        #endregion

    }

    public struct UVector4
    {
        public uint X, Y, Z, W;

        public static UVector4 Zero => new UVector4(0);

        public UVector4(uint scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public UVector4(uint x, uint y, uint z, uint w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public UVector4(UVector3 xyz, uint w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
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

        public UVector3 XYZ
        {
            get => new UVector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        #region Operators

        // Add
        public static UVector4 operator +(UVector4 a, UVector4 b)
        {
            return new UVector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        }

        public static UVector4 operator +(UVector4 a, uint scalar)
        {
            return new UVector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        public static UVector4 operator +(uint scalar, UVector4 a)
        {
            return new UVector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        // Subtract
        public static UVector4 operator -(UVector4 a, UVector4 b)
        {
            return new UVector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        }

        public static UVector4 operator -(UVector4 a, uint scalar)
        {
            return new UVector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        public static UVector4 operator -(uint scalar, UVector4 a)
        {
            return new UVector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        // Multiply
        public static UVector4 operator *(UVector4 a, UVector4 b)
        {
            return new UVector4(a.X * b.X, a.Y * b.Y, a.Z * b.Z, a.W * b.W);
        }

        public static UVector4 operator *(UVector4 vector, uint scalar)
        {
            return new UVector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        public static UVector4 operator *(uint scalar, UVector4 vector)
        {
            return new UVector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        // Divide
        public static UVector4 operator /(UVector4 a, UVector4 b)
        {
            return new UVector4(a.X / b.X, a.Y / b.Y, a.Z / b.Z, a.W / b.W);
        }

        public static UVector4 operator /(UVector4 vector, uint scalar)
        {
            return new UVector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        public static UVector4 operator /(uint scalar, UVector4 vector)
        {
            return new UVector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        // Equality
        public static bool operator ==(UVector4 a, UVector4 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;
        }

        public static bool operator !=(UVector4 a, UVector4 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is UVector4 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z, W);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"UVector4({X}, {Y}, {Z}, {W})";
        }

        #endregion

    }

    public struct DVector4
    {
        public double X, Y, Z, W;

        public static DVector4 Zero => new DVector4(0.0);

        public DVector4(double scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public DVector4(double x, double y, double z, double w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public DVector4(DVector3 xyz, double w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
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

        public DVector3 XYZ
        {
            get => new DVector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        #region Operators

        // Add
        public static DVector4 operator +(DVector4 a, DVector4 b)
        {
            return new DVector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
        }

        public static DVector4 operator +(DVector4 a, double scalar)
        {
            return new DVector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        public static DVector4 operator +(double scalar, DVector4 a)
        {
            return new DVector4(a.X + scalar, a.Y + scalar, a.Z + scalar, a.W + scalar);
        }

        // Subtract
        public static DVector4 operator -(DVector4 a, DVector4 b)
        {
            return new DVector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
        }

        public static DVector4 operator -(DVector4 a, double scalar)
        {
            return new DVector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        public static DVector4 operator -(double scalar, DVector4 a)
        {
            return new DVector4(a.X - scalar, a.Y - scalar, a.Z - scalar, a.W - scalar);
        }

        // Multiply
        public static DVector4 operator *(DVector4 a, DVector4 b)
        {
            return new DVector4(a.X * b.X, a.Y * b.Y, a.Z * b.Z, a.W * b.W);
        }

        public static DVector4 operator *(DVector4 vector, double scalar)
        {
            return new DVector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        public static DVector4 operator *(double scalar, DVector4 vector)
        {
            return new DVector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
        }

        // Divide
        public static DVector4 operator /(DVector4 a, DVector4 b)
        {
            return new DVector4(a.X / b.X, a.Y / b.Y, a.Z / b.Z, a.W / b.W);
        }

        public static DVector4 operator /(DVector4 vector, double scalar)
        {
            return new DVector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        public static DVector4 operator /(double scalar, DVector4 vector)
        {
            return new DVector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
        }

        // Equality
        public static bool operator ==(DVector4 a, DVector4 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;
        }

        public static bool operator !=(DVector4 a, DVector4 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is DVector4 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z, W);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"DVector4({X}, {Y}, {Z}, {W})";
        }

        #endregion

    }

    public struct BVector4
    {
        public bool X, Y, Z, W;

        public static BVector4 Zero => new BVector4(false);

        public BVector4(bool scalar)
        {
            X = scalar;
            Y = scalar;
            Z = scalar;
            W = scalar;
        }

        public BVector4(bool x, bool y, bool z, bool w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public BVector4(BVector3 xyz, bool w)
        {
            X = xyz.X;
            Y = xyz.Y;
            Z = xyz.Z;
            W = w;
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

        public BVector3 XYZ
        {
            get => new BVector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        #region Operators

        // Equality
        public static bool operator ==(BVector4 a, BVector4 b)
        {
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W;
        }

        public static bool operator !=(BVector4 a, BVector4 b)
        {
            return !(a == b);
        }

        #endregion

        #region Methods

        // Override Equals method
        public override readonly bool Equals(object? obj)
        {
            if (obj is BVector4 other)
            {
                return this == other;
            }
            return false;
        }

        // Override GetHashCode
        public override readonly int GetHashCode()
        {
            return HashCode.Combine(X, Y, Z, W);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"BVector4({X}, {Y}, {Z}, {W})";
        }

        #endregion

    }

}