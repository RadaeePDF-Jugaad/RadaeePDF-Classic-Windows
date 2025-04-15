using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RDPDFLib.pdf;
using System.IO;
using Windows.Security.Cryptography;
using Windows.Security.Cryptography.Core;
using Windows.Storage.Streams;

namespace RDPDFLib.util
{
    class PDFStreamWrap : PDFStream
    {
        private Stream m_stream = null;
        private bool m_fix = false;
        public void Open(Stream stream)
        {
            if (stream != null && stream.CanSeek && stream.CanRead)
            {
                m_stream = stream;
                string type_name = m_stream.GetType().ToString();
                m_fix = (type_name.LastIndexOf("NativeFileStream") > 0);
            }
        }
        public virtual void Close()
        {
            if (m_stream != null)
            {
                m_stream.Dispose();
                m_stream = null;
            }
        }
        public virtual void Flush()
        {
            if (m_stream != null)
                m_stream.Flush();
        }
        public virtual long GetLength()
        {
            if (m_stream == null) return 0;
            return m_stream.Length;
        }
        public virtual long GetPosition()
        {
            if (m_stream == null) return 0;
            return m_stream.Position;
        }
        public virtual int Read(byte[] buf)
        {
            if (m_stream == null) return 0;
            int read = m_stream.Read(buf, 0, buf.Length);
            return read;
        }
        public virtual bool SetPosition(long pos)
        {
            if (m_stream == null) return false;
            string type_name = m_stream.GetType().ToString();
            if (m_fix)
            {
                ulong uoffset = (ulong)pos;
                ulong fix = ((uoffset & 0xffffffffL) << 32) | ((uoffset & 0xffffffff00000000L) >> 32);
                m_stream.Seek((long)fix, SeekOrigin.Begin);
            }
            else
                m_stream.Seek(pos, SeekOrigin.Begin);
            return true;
        }
        public virtual int Write(byte[] buf)
        {
            if (m_stream == null) return 0;
            long prev = m_stream.Position;
            m_stream.Write(buf, 0, buf.Length);
            return (int)(m_stream.Position - prev);
        }
        public virtual bool Writeable()
        {
            if (m_stream == null) return false;
            return m_stream.CanWrite;
        }
    }
    class PDFAESStream : PDFStream
    {
        static private int BLOCK_ENC_SIZE = 4096;
        static private int BLOCK_DEC_SIZE = BLOCK_ENC_SIZE - 16;
        private Stream m_stream = null;
        private byte[] m_dec_block = null;
        private int m_dec_block_len = 0;
        private int m_dec_pos;
        private int m_dec_len;
        private int m_enc_len;
        private bool m_flush = true;
        private SymmetricKeyAlgorithmProvider m_algorithm = null;
        private CryptographicKey m_key = null;
        private IBuffer m_iv = null;
        private bool m_fix = false;
        /// <summary>
        /// open AES stream.
        /// if length of stream is zero, process it as create, and stream must be writeable.
        /// if length of stream is not zero:
        ///    if stream is writeable, can read/write stream.
        ///    if stream is readonly, can read from stream.
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="key">16 bytes length bytes array for key</param>
        /// <returns>true or false</returns>
        public bool Open(Stream stream, byte[] key)
        {
            if (key.Length < 16 || stream == null || !stream.CanSeek || !stream.CanRead) return false;
            m_stream = stream;
            m_algorithm = SymmetricKeyAlgorithmProvider.OpenAlgorithm("AES_CBC_PKCS7");
            string type_name = m_stream.GetType().ToString();
            m_fix = (type_name.LastIndexOf("NativeFileStream") > 0);
            byte[] ivbytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
            IBuffer keybuf = CryptographicBuffer.CreateFromByteArray(key);
            m_iv = CryptographicBuffer.CreateFromByteArray(ivbytes);
            m_key = m_algorithm.CreateSymmetricKey(keybuf);
            m_enc_len = (int)stream.Length;
            m_dec_pos = 0;
            if (m_enc_len == 0)//create
            {
                if (!stream.CanWrite) return false;
                m_dec_len = 0;
            }
            else
            {
                if( m_enc_len % BLOCK_ENC_SIZE != 4 )//check
                {
                    return false;
                }
                strSetPos(m_enc_len - 4);
                m_dec_len = strReadInt();
            }
            return true;
        }

        private bool dec_block()
        {
            int block = m_dec_pos / BLOCK_DEC_SIZE;
            if (block < 0 || block * BLOCK_ENC_SIZE >= m_enc_len - 4)
            {
                m_dec_block = null;
                m_dec_block_len = 0;
                return false;
            }
            try
            {
                int len = BLOCK_ENC_SIZE;
                if ((block + 1) * BLOCK_DEC_SIZE > m_dec_len)
                    len = (m_dec_len - block * BLOCK_DEC_SIZE + 16) & (~15);//calculate padding length
                strSetPos(block * BLOCK_ENC_SIZE);
                byte[] src = new byte[len];
                len = m_stream.Read(src, 0, len);
                if (len <= 0)
                {
                    m_dec_block = null;
                    m_dec_block_len = 0;
                    return false;
                }
                IBuffer src_buf = CryptographicBuffer.CreateFromByteArray(src);
                IBuffer dec_buf = CryptographicEngine.Decrypt(m_key, src_buf, m_iv);
                CryptographicBuffer.CopyToByteArray(dec_buf, out src);
                m_dec_block_len = src.Length;
                if (src.Length == BLOCK_DEC_SIZE)
                    m_dec_block = src;
                else
                {
                    m_dec_block = new byte[BLOCK_DEC_SIZE];
                    Array.Copy(src, m_dec_block, src.Length);
                }
                return true;
            }
            catch (Exception)
            {
                m_dec_block = null;
                m_dec_block_len = 0;
                return false;
            }
        }
        private bool enc_block()
        {
            int block = m_dec_pos / BLOCK_DEC_SIZE;
            try
            {
                byte[] src = new byte[m_dec_block_len];
                Array.Copy(m_dec_block, src, m_dec_block_len);
                IBuffer src_buf = CryptographicBuffer.CreateFromByteArray(src);
                IBuffer enc_buf = CryptographicEngine.Encrypt(m_key, src_buf, m_iv);
                byte[] enc;
                CryptographicBuffer.CopyToByteArray(enc_buf, out enc);
                strSetPos(block * BLOCK_ENC_SIZE);
                if (enc.Length != 4096)
                {
                    byte[] pad_data = new byte[4096];
                    Array.Copy(enc, pad_data, enc.Length);
                    m_stream.Write(pad_data, 0, 4096);
                }
                else
                    m_stream.Write(enc, 0, 4096);
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
        public virtual void Close()
        {
            if (!m_flush)
            {
                enc_block();
                Flush();
                m_flush = true;
            }
            if (m_stream != null)
            {
                m_stream.Dispose();
                m_stream = null;
            }
            m_enc_len = 0;
            m_dec_len = 0;
            m_dec_block = null;
            m_dec_block_len = 0;
            m_dec_pos = 0;
        }
        public virtual void Flush()
        {
            if (m_stream != null)
                m_stream.Flush();
        }
        public virtual long GetLength()
        {
            return m_dec_len;
        }
        public virtual long GetPosition()
        {
            return m_dec_pos;
        }
        public virtual int Read(byte[] buf)
        {
            if (m_dec_block == null) return 0;
            if (!m_flush)
            {
                enc_block();
                m_flush = true;
            }
            int off = m_dec_pos % BLOCK_DEC_SIZE;
            int total = buf.Length;
            if (m_dec_pos + total > m_dec_len)
                total = m_dec_len - m_dec_pos;
            int read = 0;
            while (read < total)
            {
                int len = m_dec_block_len - off;
                if (len > total - read) len = total - read;
                Array.Copy(m_dec_block, off, buf, read, len);
                off = 0;
                m_dec_pos += len;
                read += len;
                if (m_dec_pos % BLOCK_DEC_SIZE == 0)
                    dec_block();
            }
            return read;
        }
        private int strReadInt()
        {
            byte[] tmp = new byte[4];
            m_stream.Read(tmp, 0, 4);
            return (((int)tmp[3]) << 24) | (((int)tmp[2]) << 16) | (((int)tmp[1]) << 8) | (tmp[0]);
        }
        private void strWriteInt(int v)
        {
            byte[] tmp = new byte[4];
            tmp[0] = (byte)(v & 0xFF);
            tmp[1] = (byte)((v >> 8) & 0xFF);
            tmp[2] = (byte)((v >> 16) & 0xFF);
            tmp[3] = (byte)((v >> 24) & 0xFF);
            m_stream.Write(tmp, 0, 4);
        }
        private bool strSetPos(long pos)
        {
            if (m_stream == null) return false;
            string type_name = m_stream.GetType().ToString();
            if (m_fix)
            {
                ulong uoffset = (ulong)pos;
                ulong fix = ((uoffset & 0xffffffffL) << 32) | ((uoffset & 0xffffffff00000000L) >> 32);
                m_stream.Seek((long)fix, SeekOrigin.Begin);
            }
            else
                m_stream.Seek(pos, SeekOrigin.Begin);
            return true;
        }
        public virtual bool SetPosition(long pos)
        {
            if (pos < 0) pos = 0;
            if (pos > m_dec_len) pos = m_dec_len;
            if (!m_flush)
            {
                enc_block();
                m_flush = true;
            }
            int block_old = m_dec_pos / BLOCK_DEC_SIZE;
            int block_cur = (int)pos / BLOCK_DEC_SIZE;
            m_dec_pos = (int)pos;
            if (block_cur == block_old)
                return false;
            dec_block();
            return true;
        }
        public virtual int Write(byte[] buf)
        {
            if (!m_stream.CanWrite) return 0;
            int off = m_dec_pos % BLOCK_DEC_SIZE;
            int total = buf.Length;
            int written = 0;
            while (written < total)
            {
                if (m_dec_block == null) m_dec_block = new byte[BLOCK_DEC_SIZE];
                int len = BLOCK_DEC_SIZE - off;
                if (len > total - written) len = total - written;
                if (m_dec_block_len < off + len) m_dec_block_len = off + len;
                Array.Copy(buf, written, m_dec_block, off, len);
                enc_block();
                off = 0;
                m_dec_pos += len;
                written += len;
                if (m_dec_pos % BLOCK_DEC_SIZE == 0)
                    dec_block();
            }
            if (m_dec_pos > m_dec_len)
            {
                m_dec_len = m_dec_pos;
                try
                {
                    m_enc_len = (int)m_stream.Length;
                    if (m_enc_len % BLOCK_ENC_SIZE == 4)
                        strSetPos(m_enc_len - 4);
                    else
                        strSetPos(m_enc_len);
                    strWriteInt(m_dec_len);
                    m_enc_len = (int)m_stream.Length;
                }
                catch (Exception)
                {
                }
            }
            m_flush = (m_dec_pos % BLOCK_DEC_SIZE != 0);
            return written;
        }
        public virtual bool Writeable()
        {
            if (m_stream == null) return false;
            return m_stream.CanWrite;
        }
    }
}
