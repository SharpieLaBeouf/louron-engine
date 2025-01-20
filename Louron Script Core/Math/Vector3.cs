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
        public override readonly bool Equals(object obj)
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

    public struct BVector3
    {
        public bool X, Y, Z;

        public static BVector3 False => new BVector3(false);
        public static BVector3 True => new BVector3(true);

        public BVector3(bool value)
        {
            X = value;
            Y = value;
            Z = value;
        }

        public BVector3(bool x, bool y, bool z)
        {
            X = x;
            Y = y;
            Z = z;
        }
    }
}
