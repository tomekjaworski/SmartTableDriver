using System;

namespace Newtonsoft.Json.Converters
{
    public sealed class HexStringJsonConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType) => typeof(int).Equals(objectType);

        public override bool CanRead => true;
        public override bool CanWrite => false;

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer) => throw new NotImplementedException();

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            string str = reader.Value as string;
            if (string.IsNullOrEmpty(str) || !str.StartsWith("0x"))
                throw new JsonSerializationException($"Unable to convert string to integer");
            return Convert.ToInt32(str, 16);
        }
    }
}
