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
            return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
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
        public override readonly bool Equals(object obj)
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
            return HashCode.Combine(X, Y, Z);
        }

        // Normalize function
        public readonly Vector4 Normalize()
        {
            float length = (float)Math.Sqrt(X * X + Y * Y + Z * Z);
            if (length > 0)
            {
                return this * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Static Normalize function
        public static Vector4 Normalize(Vector4 v)
        {
            float length = (float)Math.Sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
            if (length > 0)
            {
                return v * (1.0f / length);
            }
            return Zero; // Return zero vector if length is zero to avoid division by zero
        }

        // Transform function: Applies a quaternion rotation to the vector
        public static Vector4 Transform(Vector4 vector, Quaternion quaternion)
        {
            // Quaternion * Vector4 * Quaternion.Inverse (Rotation transformation)
            Quaternion qv = new Quaternion(vector.X, vector.Y, vector.Z, 0);
            Quaternion qConjugate = new Quaternion(-quaternion.X, -quaternion.Y, -quaternion.Z, quaternion.W);

            Quaternion result = quaternion * qv * qConjugate;

            return new Vector4(result.X, result.Y, result.Z, result.W);
        }

        // Overriding the ToString method
        public override readonly string ToString()
        {
            return $"Vector4({X}, {Y}, {Z})";
        }

        #endregion

    }
    public struct BVector4
    {
        public bool X, Y, Z, W;

        public static BVector4 False => new BVector4(false);
        public static BVector4 True => new BVector4(true);

        public BVector4(bool value)
        {
            X = value;
            Y = value;
            Z = value;
            W = value;
        }

        public BVector4(bool x, bool y, bool z, bool w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }
    }
}