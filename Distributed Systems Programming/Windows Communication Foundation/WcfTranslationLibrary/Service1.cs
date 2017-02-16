namespace WcfTranslationLibrary
{
    public class TranslationService : ITranslationService
    {
        public string Translate(string value)
        {
            return string.Format("You entered: {0}", value);
        }
    }
}
